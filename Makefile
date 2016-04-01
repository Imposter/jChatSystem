# GNU Make workspace makefile autogenerated by Premake

.NOTPARALLEL:

ifndef config
  config=debug_linux32
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug_linux32)
  jchat_config = debug_linux32
  jchat_server_config = debug_linux32
  jchat_client_config = debug_linux32
endif
ifeq ($(config),debug_linux64)
  jchat_config = debug_linux64
  jchat_server_config = debug_linux64
  jchat_client_config = debug_linux64
endif
ifeq ($(config),debug_osx32)
  jchat_config = debug_osx32
  jchat_server_config = debug_osx32
  jchat_client_config = debug_osx32
endif
ifeq ($(config),debug_osx64)
  jchat_config = debug_osx64
  jchat_server_config = debug_osx64
  jchat_client_config = debug_osx64
endif
ifeq ($(config),debug_win32)
  jchat_config = debug_win32
  jchat_server_config = debug_win32
  jchat_client_config = debug_win32
endif
ifeq ($(config),debug_win64)
  jchat_config = debug_win64
  jchat_server_config = debug_win64
  jchat_client_config = debug_win64
endif
ifeq ($(config),release_linux32)
  jchat_config = release_linux32
  jchat_server_config = release_linux32
  jchat_client_config = release_linux32
endif
ifeq ($(config),release_linux64)
  jchat_config = release_linux64
  jchat_server_config = release_linux64
  jchat_client_config = release_linux64
endif
ifeq ($(config),release_osx32)
  jchat_config = release_osx32
  jchat_server_config = release_osx32
  jchat_client_config = release_osx32
endif
ifeq ($(config),release_osx64)
  jchat_config = release_osx64
  jchat_server_config = release_osx64
  jchat_client_config = release_osx64
endif
ifeq ($(config),release_win32)
  jchat_config = release_win32
  jchat_server_config = release_win32
  jchat_client_config = release_win32
endif
ifeq ($(config),release_win64)
  jchat_config = release_win64
  jchat_server_config = release_win64
  jchat_client_config = release_win64
endif

PROJECTS := jchat jchat_server jchat_client

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

jchat:
ifneq (,$(jchat_config))
	@echo "==== Building jchat ($(jchat_config)) ===="
	@${MAKE} --no-print-directory -C . -f jchat.make config=$(jchat_config)
endif

jchat_server:
ifneq (,$(jchat_server_config))
	@echo "==== Building jchat_server ($(jchat_server_config)) ===="
	@${MAKE} --no-print-directory -C . -f jchat_server.make config=$(jchat_server_config)
endif

jchat_client:
ifneq (,$(jchat_client_config))
	@echo "==== Building jchat_client ($(jchat_client_config)) ===="
	@${MAKE} --no-print-directory -C . -f jchat_client.make config=$(jchat_client_config)
endif

clean:
	@${MAKE} --no-print-directory -C . -f jchat.make clean
	@${MAKE} --no-print-directory -C . -f jchat_server.make clean
	@${MAKE} --no-print-directory -C . -f jchat_client.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  debug_linux32"
	@echo "  debug_linux64"
	@echo "  debug_osx32"
	@echo "  debug_osx64"
	@echo "  debug_win32"
	@echo "  debug_win64"
	@echo "  release_linux32"
	@echo "  release_linux64"
	@echo "  release_osx32"
	@echo "  release_osx64"
	@echo "  release_win32"
	@echo "  release_win64"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   jchat"
	@echo "   jchat_server"
	@echo "   jchat_client"
	@echo ""
	@echo "For more information, see http://industriousone.com/premake/quick-start"