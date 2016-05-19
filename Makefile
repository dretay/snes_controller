PROTOC := ./nanopb-0.3.5/generator-bin/protoc.exe
COPY := cp
GIT := git
PROTO_DIR := common
PROTO_FILES := $(wildcard $(PROTO_DIR)/*.proto)
COMPILED_PROTO_FILES := $(wildcard $(PROTO_DIR)/$(PROTO_DIR)/*)

default: protoc copy_sender copy_receiver

protoc:
	$(GIT) submodule update --remote --merge
	$(foreach proto,$(PROTO_FILES), $(PROTOC) --nanopb_out=./$(PROTO_DIR) ./$(proto); )
	# $(foreach proto,$(COMPILED_PROTO_FILES), $(COPY) $(proto)  $(shell pwd;); )

copy_sender:
	$(foreach proto,$(COMPILED_PROTO_FILES), $(COPY) $(proto)  $(shell pwd;)/snes_sender; )

copy_receiver:
	$(foreach proto,$(COMPILED_PROTO_FILES), $(COPY) $(proto)  $(shell pwd;)/snes_receiver; )
