package main

import (
	"errors"
	"flag"
	"fmt"
	"log"
	"time"

	"github.com/jinzhu/gorm"
	_ "github.com/jinzhu/gorm/dialects/postgres"

	"encoding/json"

	"gitlab.com/smartmakers/sm-lora-core/domain"
	"gitlab.com/smartmakers/sm-lora-core/domain/types"
)

const (
	DEFAULT_DB_DIALECT = "postgres"
	DEFAULT_DB_DSN     = "host=prod.smartmakers.de user=sm_lora_core dbname=sm_lora_core password=sm-lora-core-pass"
)

var (
	withoutHeader bool
	sinceString   string
	untilString   string
	dbDialect     string
	dbDSN         string
	dbTable       string
)

const InputTimeLayout = "2006/01/02 15:04:05"

func init() {
	now := time.Now()
	since := time.Date(now.Year(), now.Month(), now.Day(), 0, 1, 0, 0, now.Location())
	until := since.Add(23*time.Hour + 58*time.Minute)

	flag.BoolVar(&withoutHeader, "without-header", false, "Do not write the CSV header")
	flag.StringVar(&sinceString, "since", since.Format(InputTimeLayout), "Since when to import the data")
	flag.StringVar(&untilString, "until", until.Format(InputTimeLayout), "Until when to import the data")
	flag.StringVar(&dbDialect, "dialect", DEFAULT_DB_DIALECT, "Database dialect")
	flag.StringVar(&dbDSN, "dsn", DEFAULT_DB_DSN, "Database DSN")
	flag.StringVar(&dbTable, "table", "db_integration__feather_coverage_testers", "Database table name")
}

func main() {
	flag.Parse()

	// Parse since/until time.
	since, err := time.ParseInLocation(InputTimeLayout, sinceString, time.Local)
	if err != nil {
		log.Fatal(err)
	}
	until, err := time.ParseInLocation(InputTimeLayout, untilString, time.Local)
	if err != nil {
		log.Fatal(err)
	}

	// Open database.
	db, err := gorm.Open(dbDialect, dbDSN)
	if err != nil {
		log.Fatal("Failed to connect to the database: %s", err)
	}

	db = db. // Period filter
			Where("reception_time >= ?", since.UTC().Format("%2006-01-02T15:04:05%")).
			Where("reception_time <= ?", until.UTC().Format("%2006-01-02T15:04:05%"))

	// Fetch entries from database.
	var uplinks []*uplinkEntry
	if err = db.Find(&uplinks).Error; err != nil {
		log.Fatal(err)
	}

	if !withoutHeader {

		// Write csv headers to output.
		fmt.Println("" +
			"uplink_id," +
			"uplink_frequency," +
			"uplink_modulation," +
			"uplink_data_rate," +
			"uplink_coding_rate," +
			"reception_unix," +
			"reception_date," +
			"reception_time," +
			"reception_rssi," +
			"reception_snr," +
			"reception_gateway_eui," +
			"device_id," +
			"device_name," +
			"device_group_id," +
			"device_group_name," +
			"device_otaa_app_eui," +
			"device_otaa_dev_eui," +
			"device_session_id," +
			"device_session_f_cnt_up," +
			"device_session_f_cnt_down,")
	}

	for _, uplink := range uplinks {

		// Unmarhsal json of receptions.
		var rxs []*domain.ReceptionInformation
		if err = json.Unmarshal([]byte(uplink.ReceptionsJSON), &rxs); err != nil {
			log.Fatal(err)
		}

		for _, rx := range rxs {

			// Print row to output.
			fmt.Printf("%d,%d,%s,%s,%s,%d,%s,%s,%d,%.2f,%s,%d,%s,%d,%s,%s,%s,%d,%d,%d\n",
				uplink.ID,
				uplink.Frequency,
				uplink.Modulation,
				uplink.DataRate,
				uplink.CodingRate,
				rx.Time.UnixNano(),
				rx.Time.Local().Format("2006/01/02"),
				rx.Time.Local().Format("15:04:05.000"),
				rx.RSSI,
				*rx.SNR,
				rx.GatewayEUI,
				uplink.DeviceID,
				uplink.DeviceName,
				uplink.DeviceGroupID,
				uplink.DeviceGroupName,
				uplink.DeviceOTAAAppEUI,
				uplink.DeviceOTAADevEUI,
				uplink.DeviceSessionID,
				uplink.DeviceSessionFrameCountUp,
				uplink.DeviceSessionFrameCountDown,
			)
		}
	}
}

// uplink entry in the DB (DTO).
type uplinkEntry struct {
	ID int

	// Device info
	DeviceID   int
	DeviceName string

	// Group info
	DeviceGroupID   int
	DeviceGroupName string

	// OTAA Parameters
	DeviceOTAADevEUI types.EUI64 `gorm:"column:device_otaa_dev_eui;type:varchar(16)"`
	DeviceOTAAAppEUI types.EUI64 `gorm:"column:device_otaa_app_eui;type:varchar(16)"`

	// Session info.
	DeviceSessionID             int
	DeviceSessionDevAddr        types.DevAddr `gorm:"type:varchar(8)"` // ABP activation
	DeviceSessionFrameCountUp   int
	DeviceSessionFrameCountDown int

	// Gateway info.
	GatewayID   int
	GatewayEUI  types.EUI64 `gorm:"column:gateway_eui;type:varchar(16)"`
	GatewayName string

	Port          int // FPort
	Confirmed     bool
	PayloadRaw    string `gorm:"size:1024"` // FRMPayload
	PayloadFields string `gorm:"size:1024"` // Decoded Payload

	// Receptions.
	ReceptionsCount int
	ReceptionsJSON  string `gorm:"column:receptions_json"`

	// Radio parameters.
	Frequency  int
	Modulation string
	DataRate   string
	CodingRate string
}

func (row *uplinkEntry) TableName() string {
	return dbTable
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
