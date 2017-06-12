.PHONY: build

build:
	@mkdir -p bin
	@echo "Compiling sources for linux (amd64)"
	@GOOS=linux GOARCH=amd64 go build -o bin/feather-import cmd/feather-import/feather-import.go
	@echo "Compiling sources for windows (amd64)"
	@GOOS=windows GOARCH=amd64 go build -o bin/feather-import.exe cmd/feather-import/feather-import.go