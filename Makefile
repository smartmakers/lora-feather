.PHONY: build

build:
	@mkdir -p build
	@echo "Compiling sources for linux (amd64)"
	@GOOS=linux GOARCH=amd64 go build -o build/feather-import_linux cmd/feather-import/*.go
	@GOOS=linux GOARCH=amd64 go build -o build/generate-device-identifiers_linux cmd/generate-device-identifiers/*.go
	@echo "Compiling sources for windows (amd64)"
	@GOOS=windows GOARCH=amd64 go build -o build/feather-import_windows.exe cmd/feather-import/*.go
	@GOOS=windows GOARCH=amd64 go build -o build/generate-device-identifiers_windows.exe cmd/generate-device-identifiers/*.go