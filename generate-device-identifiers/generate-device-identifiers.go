package main

import (
	"encoding/csv"
	"flag"
	"io"
	"os"
	"path/filepath"

	"fmt"
	"log"
	"strings"
	"text/template"
)

const hppSkeleton = `#include <lmic.h>
#include <hal/hal.h>

// DEVEUI: Unique device ID (LSBF)
static const u1_t DEVEUI[8] PROGMEM = { {{join .DEVEUI}} };

// APPEUI: Application ID (LSBF)
static const u1_t APPEUI[8] PROGMEM = { {{join .APPEUI}} };

// APPKEY: Device-specific AES key.
static const u1_t APPKEY[16] PROGMEM = { {{join .APPKEY}} };
`

var (
	devicesPath string
	outDir      string
)

func main() {
	parseFlags()

	os.Mkdir(outDir, 0666)

	devices := readCsv(devicesPath)
	headerTmpl := createHeaderTemplate()

	for _, device := range devices {
		generateHeader(headerTmpl, device)
	}

	log.Println("done!")
}

func parseFlags() {
	wd, err := os.Getwd()
	check(err)

	flag.StringVar(&devicesPath, "devices", filepath.Join(wd, "devices.csv"), "file with Device configurations")
	flag.StringVar(&outDir, "out", filepath.Join(wd, "embedded", "DeviceIdentifiers"), "output directory for generated header files")

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

func generateHeader(headerTmpl *template.Template, device []string) {
	filename := device[0] + ".h"

	f, err := os.OpenFile(filepath.Join(outDir, filename), os.O_WRONLY|os.O_CREATE, 0666)
	check(err)
	defer f.Close()

	log.Printf("processing \"%s\"", filename)

	deveui := bytesToHexSlice(device[1], true)
	appeui := bytesToHexSlice(device[2], true)
	appkey := bytesToHexSlice(device[3], true)

	data := struct {
		DEVEUI []string
		APPEUI []string
		APPKEY []string
	}{
		DEVEUI: deveui,
		APPEUI: appeui,
		APPKEY: appkey,
	}

	err = headerTmpl.Execute(f, data)
	check(err)
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

func check(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
