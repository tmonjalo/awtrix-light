ENV ?= ulanzi

build:
	pio run -e ${ENV}

flash:
	pio run -e ${ENV} -t upload

serial:
	pio device monitor -e ${ENV} --quiet --no-reconnect
