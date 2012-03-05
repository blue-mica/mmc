SRC_DIR=src
BUILD_DIR=build
OUT_DIR=.

# ================ START =================
BIN_FRAGMENT_CLASSIFIER=$(OUT_DIR)/libfragment_classifier.so
OBJ_FRAGMENT_CLASSIFIER= $(BUILD_DIR)/fragment_classifier.o \
			 $(BUILD_DIR)/minIni.o \
			 $(BUILD_DIR)/entropy/entropy.o
CFLAGS_FRAGMENT_CLASSIFIER=$(CFLAGS) -fPIC -Iinclude/entropy
LDFLAGS_FRAGMENT_CLASSIFIER=-shared -Wl,-soname, -lm
# ================= END ==================

# ================ START DATA SNIFFER =================
LDFLAGS_DATA_SNIFFER=-L. -lfragment_classifier
OBJ_DATA_SNIFFER=$(BUILD_DIR)/data_sniffer.o
BIN_DATA_SNIFFER=$(OUT_DIR)/data_sniffer
# ================= END DATA SNIFFER ==================
