package main

import (
	"errors"
	"flag"
	"fmt"
	"log"
	"time"

	"gitlab.com/smartmakers/adeunis/database"
)

const (
	DEFAULT_DB_DIALECT = "postgres"
	DEFAULT_DB_DSN     = "host=production.smartmakers.de user=loraserver dbname=loraserver sslmode=disable password=dbpassword"
)

var (
	withoutHeader bool
	dateString    string
	dbDialect     string
	dbDSN         string
	appEUI        string
)

func init() {
	flag.BoolVar(&withoutHeader, "without-header", false, "Do not write the CSV header")
	flag.StringVar(&dateString, "date", time.Now().Format("02/01/2006"), "Date to import the data from")
	flag.StringVar(&dbDialect, "dialect", DEFAULT_DB_DIALECT, "Database dialect")
	flag.StringVar(&dbDSN, "dsn", DEFAULT_DB_DSN, "Database DSN")
	flag.StringVar(&appEUI, "appeui", "0000000000000001", "AppEUI of Feather devices")
}

// Application payload.
type Payload struct {

	// Battery voltage at the transmission time.
	Voltage float64

	// RSSI of previous downlink.
	RSSI int
}

func (payload *Payload) UnmarshalBinary(data []byte) error {

	if len(data) < 2 {
		return errors.New("expected 2 bytes of data.")
	}

	// Parse battery level.
	payload.Voltage = float64(uint16(data[0])<<3) * 3.3 / 1024

	// Parse RSSI value.
	payload.RSSI = int(int8(data[1]) - 64) // LMIC radio.c (l.786), RFM95 (5.5.5, p82)

	return nil
}

func main() {
	flag.Parse()

	// Parse data day.
	date, err := time.Parse("02/01/2006", dateString)
	if err != nil {
		log.Fatal(err)
	}

	// Fetch messages from database.
	rows, err := database.Import(dbDialect, dbDSN, []string{appEUI}, nil, date)
	if err != nil {
		log.Fatal("Error fetching from database: ", err)
	}

	if !withoutHeader {

		// Write csv headers to output.
		fmt.Println("Time, GatewayEUI, AppEUI, DevEUI, UplinkRSSI, DownlinkRSSI, BatteryVoltage")
	}

	for _, row := range rows {

		// Unmarhsal adeunis binary payload.
		var decoded Payload
		err := decoded.UnmarshalBinary(row.Data)
		if err != nil {
			log.Fatal("Error decoding adeunis payload: ", err)
		}

		// Print row to output.
		fmt.Printf("%s,%s,%s,%s,%d,%d,%f\n",
			row.Time,
			row.GwEUI,
			row.AppEUI,
			row.DevEUI,
			row.RSSI,
			decoded.RSSI,
			decoded.Voltage)
	}
}
