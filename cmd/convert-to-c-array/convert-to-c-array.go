package main

import (
	"os"
	logger "github.com/Sirupsen/logrus"
	"fmt"
)

// this prints an eui as a C array (euis are inverted in memory)
func convertEUI(eui string) (result string) {

	if len(eui) != 16 {
		logger.Fatalf("EUI needs exactly 16 hex characters, but had %v", len(eui))
	}

	for i := 0; i != len(eui)/2; i++ {
		slice := eui[2*i:2*i+2]

		if len(result) == 0 {
			result = "0x" + slice + " }"
		} else {

			result = "0x" + slice + ", " + result
		}
	}

	result = "{ " + result

	return result
}

func convertAppKey(appkey string) (result string) {

	if len(appkey) != 32 {
		logger.Fatalf("AppKey needs exactly 16 hex characters, but had %v", len(appkey))
	}

	for i := 0; i != len(appkey)/2; i++ {
		slice := appkey[2*i:2*i+2]

		if len(result) == 0 {
			result = "{ " + "0x" + slice
		} else {

			result =  result + ", 0x" + slice
		}
	}

	result = result + " }"

	return result
}

// converts an eui to a c array (and reverses it)
func main() {
	switch len(os.Args) {
	case 2:
		fmt.Println(convertEUI(os.Args[1]))
	case 4:
		fmt.Println(convertEUI(os.Args[1]))
		fmt.Println(convertEUI(os.Args[2]))
		fmt.Println(convertAppKey(os.Args[3]))
	default:
		logger.Fatal("Unexpected number of arguments")
	}
}
