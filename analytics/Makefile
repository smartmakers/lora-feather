.PHONY: build

build:
	@mkdir -p build
	@echo "Compiling sources for linux (amd64)"
	@GOOS=linux GOARCH=amd64 go build -o build/lora-feather-import cmd/import/import.go
	@echo "Compiling sources for windows (amd64)"
	@GOOS=windows GOARCH=amd64 go build -o build/lora-feather-import.exe cmd/import/import.go