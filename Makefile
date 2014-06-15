##########
# Common #
##########

# Output
OUT ?= /tmp/hdrone

# TypeScript compiler
TSC     = ./node_modules/.bin/tsc
TSFLAGS = -m commonjs -t ES5 --noImplicitAny

# Linter
TSLINT      = ./node_modules/.bin/tslint
TSLINTFLAGS =

# Definitions
TSDREPO = http://github.com/borisyankov/DefinitelyTyped/raw/master
TSDDIR  = definitions

# Definitions dependencies
TSDLIST = node     \
          smoothie

# Sources
COPTER_SOURCES = copter/master.ts          \
                 copter/flight/flight.ts   \
                 copter/vision/vision.ts   \
                 copter/contact/contact.ts

CLIENT_SOURCES = client/index.ts

###########
# Targets #
###########
.DEFAULT_GOAL = usage

$(OUT)/copter: $(shell find copter common libs -name '*.ts') config.ts
	mkdir -p $@/ && touch $@/
	$(TSC) $(TSFLAGS) $(COPTER_SOURCES) --outDir $(OUT)
	$(foreach file, $(shell find copter common libs -type f ! -name '*.ts'), $(call sync,$(file)))

$(OUT)/client: $(shell find client common libs -name '*.ts') config.ts
	mkdir -p $@/ && touch $@/
	$(TSC) $(TSFLAGS) $(CLIENT_SOURCES) --outDir $(OUT)
	ln -fs $(CURDIR)/node_modules $(OUT)/node_modules
	$(foreach file, $(shell find client common libs -type f ! -name '*.ts'), $(call sync,$(file)))

$(OUT)/npm-shrinkwrap.json: package.json
	npm shrinkwrap 2>&1 | grep -v 'Excluding' && mv npm-shrinkwrap.json $(OUT)

$(OUT)/package.json: package.json
	cp $< $@

$(OUT)/binding.gyp: binding.gyp
	cp $< $@

#########
# Tasks #
#########
.PHONY: build deploy \
        setup setup\:nm  setup\:tsd \
        update update\:nm update\:tsd \
        lint tree labels clean

usage:
	@echo "See Makefile for usage"

build: $(OUT)/copter $(OUT)/client

deploy: build $(OUT)/package.json $(OUT)/npm-shrinkwrap.json $(OUT)/binding.gyp
	cd $(OUT) && tar -cf hdrone-copter.tar $(shell ls $(OUT) | grep -vP 'client|node_modules')
	
	ssh $(RTARGET) 'rm -rf $(RPATH) && mkdir $(RPATH)'
	scp $(OUT)/hdrone-copter.tar '$(RTARGET):$(RPATH)/hdrone.tar'
	ssh $(RTARGET) 'cd $(RPATH)                                           && \
	                tar -xvf hdrone.tar > /dev/null && rm hdrone.tar      && \
                    node-gyp configure --python /bin/python2              && \
                    node-gyp build                                        && \
					cp build/Release/controller.node libs/controller.node && \
					rm -r build                                           && \
	                find . -name "*.cc" -o -name "*.hh" | xargs rm        && \
	                find . -type d -empty               | xargs rmdir'
	rm $(OUT)/hdrone-copter.tar

clone:
	tar -cf $(OUT)/hdrone.tar $(shell ls | grep -vP 'node_modules')
	scp $(OUT)/hdrone.tar '$(RTARGET):$(RPATH)/hdrone.tar'
	ssh $(RTARGET) 'cd $(RPATH) && tar -xvf hdrone.tar > /dev/null && rm hdrone.tar'

lint:
	$(foreach file, $(shell find libs copter client common -name '*.ts'), \
		$(TSLINT) -f $(file)$(\n))

setup:   setup\:nm   setup\:tsd
update: update\:nm  update\:tsd

setup\:nm:
	npm install

update\:nm:
	npm update

setup\:tsd update\:tsd:
	$(foreach name, $(TSDLIST), $(call load,$(name)) $(\n))

tree:
	@tree -CFa --dirsfirst -I '.git|node_modules|definitions|.gitignore' | head -n -2

labels:
	@egrep -Hnor --include '*.ts' '//#(TODO|FIXME|XXX):.*' libs copter client common |\
		awk -F'://#' '\
			/#FIXME:/ { print "\033[0;31m"$$2"\033[0m", "("$$1")" }\
			/#TODO:/  { print "\033[0;32m"$$2"\033[0m", "("$$1")" }\
			/#XXX:/   { print "\033[0;33m"$$2"\033[0m", "("$$1")" }\
		' | sort

clean:
	rm -rf $(OUT)/ 2> /dev/null

#############
# Auxiliary # 
#############
load = curl -L --create-dirs $(TSDREPO)/$1/$1.d.ts -o $(TSDDIR)/$1/$1.d.ts

define \n


endef

define sync
	$(shell mkdir -p $(OUT)/$(dir $1))
	$(shell cp $1 $(OUT)/$1)
endef
