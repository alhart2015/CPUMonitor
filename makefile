all: hello-world

#
# VARS
#

GO=go
RUN=run

#
# TEST
#

hello-world:
	$(GO) $(RUN) MonitorMain.go
