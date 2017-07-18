package main

import (
	"encoding/csv"
	"flag"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"log"
	"strings"
	"text/template"
	"strconv"

	"github.com/pkg/errors"
)

const hppSkeleton = `#include <lmic.h>
#include <hal/hal.h>

// OTAA/ABP: defines activation method for the device
#define {{.Mode}}

// DEVEUI: Unique device ID (LSBF)
static const u1_t DEVEUI[8] PROGMEM = { {{join .DevEUI}} };

// APPEUI: Application ID (LSBF)
static const u1_t APPEUI[8] PROGMEM = { {{join .AppEUI}} };

// APPKEY: Device-specific AES key.
static const u1_t APPKEY[16] PROGMEM = { {{join .AppKey}} };

// DEVADDR: Unique device ID
static const u4_t DEVADDR = {{.DevAddr}};

// NWKSKEY: network specific session key
static const u1_t NWKSKEY[16] PROGMEM = { {{join .NwkSKey}} };

// APPSKEY: Application specific session key
static const u1_t APPSKEY[16] PROGMEM = { {{join .AppSKey}} };
`

var (
	otaaDevicesPath string
	abpDevicesPath  string
	outDir          string
)

func main() {
	parseFlags()

	os.Mkdir(outDir, 0666)

	headerTmpl := createHeaderTemplate()

	otaaDevices := readCsv(otaaDevicesPath)
	for _, device := range otaaDevices {
		err := generateHeader(headerTmpl, device, otaa)
		check(err)
	}

	abpDevices := readCsv(abpDevicesPath)
	for _, device := range abpDevices {
		err := generateHeader(headerTmpl, device, abp)
		check(err)
	}

	log.Println("done!")
}

func parseFlags() {
	wd, err := os.Getwd()
	check(err)

	basedir := filepath.Join(wd, "..", "sm-lora-core", "tools", "init-env", "environments")
	flag.StringVar(&otaaDevicesPath, "otaa-devices", filepath.Join(basedir, "otaa-devices.csv"), "file with OTAA device configurations")
	flag.StringVar(&abpDevicesPath, "abp-devices", filepath.Join(basedir, "abp-devices.csv"), "file with ABP device configurations")
	flag.StringVar(&outDir, "out", filepath.Join(wd, "platformio", "device-identifiers"), "output directory for generated header files")

	flag.Parse()
}

// Setups CSV reader in specific manner
func csvReader(r io.Reader) *csv.Reader {
	csvReader := csv.NewReader(r)
	csvReader.Comma = ';'
	csvReader.Comment = '#'

	return csvReader
}

// Reads and filters CSV for given environment
// Mapping m is used to convert CSV entries into REST call attributes map
func readCsv(path string) [][]string {
	f, err := os.Open(path)
	check(err)

	csvReader := csvReader(f)

	records := [][]string{}
	for {
		// Read line-by-line
		record, err := csvReader.Read()
		if err == io.EOF {
			break
		}
		check(err)

		records = append(records, record)

	}

	return records
}

func createHeaderTemplate() *template.Template {

	join := func(s []string) string {
		return strings.Join(s, ", ")
	}
	funcs := template.FuncMap{"join": join}
	tmpl, err := template.New("hpp").Funcs(funcs).Parse(hppSkeleton)
	check(err)

	return tmpl
}

type activationType int;
const (
	otaa activationType = iota
	abp
)
func generateHeader(headerTmpl *template.Template, line []string, actType activationType) error {
	filename := line[0] + ".h"

	f, err := os.OpenFile(filepath.Join(outDir, filename), os.O_WRONLY|os.O_CREATE, 0666)
	if err != nil {
		return errors.Wrap(err, "cannot open output file")
	}

	defer f.Close()

	log.Printf("processing \"%s\"", filename)

	data := struct {
		Mode    string
		DevEUI  []string
		AppEUI  []string
		AppKey  []string
		DevAddr int64
		NwkSKey []string
		AppSKey []string
	}{}

	if actType == otaa {
		data.Mode = "OTAA"
		data.DevEUI = bytesToHexSlice(line[1], true)
		data.AppEUI = bytesToHexSlice(line[2], true)
		data.AppKey = bytesToHexSlice(line[3], false)
		data.DevAddr = 0
		data.NwkSKey = allZeroHexSlice(16)
		data.AppSKey = allZeroHexSlice(16)
	} else {
		devAddr, err := strconv.ParseInt(line[1], 10, 32)
		if err != nil {
			return errors.Wrap(err, "cannot parse dev addr")
		}

		data.Mode = "ABP"
		data.DevEUI = allZeroHexSlice(8)
		data.AppEUI = allZeroHexSlice(8)
		data.AppKey = allZeroHexSlice(16)
		data.DevAddr = devAddr
		data.NwkSKey = bytesToHexSlice(line[2], false)
		data.AppSKey = bytesToHexSlice(line[3], false)
	}

	return errors.Wrap(headerTmpl.Execute(f, data), "cannot instantiate header file template")
}

// converts HEX bytes string representation to slice
func bytesToHexSlice(bytes string, invert bool) []string {
	res := make([]string, 0, len(bytes)/2)

	byteWrapper := "0x%s"
	for i := 0; i != len(bytes)/2; i++ {
		b := bytes[2*i: 2*i+2]
		hexByte := fmt.Sprintf(byteWrapper, b)

		if invert {
			res = append([]string{hexByte}, res...)
		} else {
			res = append(res, hexByte)
		}
	}

	return res
}

func allZeroHexSlice(numBytes int) []string {
	str := strings.Repeat("00", numBytes)
	return bytesToHexSlice(str, false)
}

func check(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
