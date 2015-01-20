###########
# Default
###########

.PHONY : default
default :
	@echo "Please choose a target"



################
# download.bit
################

BOOTLOADER_ELF_FILE :=                  firmware/bootloader.elf
DOWNLOAD_BIT_FILE :=                    fpga/implementation/download.bit
SYSTEM_MHS_FILE :=                      fpga/system.mhs
SYSTEM_BIT_FILE :=                      fpga/implementation/system.bit
SYSTEM_BD_BMM_FILE :=                   fpga/implementation/system_bd.bmm

$(DOWNLOAD_BIT_FILE) : $(SYSTEM_MHS_FILE) $(SYSTEM_BIT_FILE) $(BOOTLOADER_ELF_FILE)
	@echo "*************************"
	@echo "* Building download.bit *"
	@echo "*************************"
	bitinit $(SYSTEM_MHS_FILE) -lp . -pe microblaze_0 $(BOOTLOADER_ELF_FILE) -bm $(SYSTEM_BD_BMM_FILE) -bt $(SYSTEM_BIT_FILE) -o $@
	@rm *.log
	@echo ""




###############################
# FPGA (+bootloader) MCS File
###############################

FPGA_MCS_FILE :=                        herbert_fpga.mcs
FPGA_MCS_VERSION_FILE :=                $(FPGA_MCS_FILE).ver
FPGA_MCS_MD5_FILE :=                    $(FPGA_MCS_FILE).md5
FPGA_MCS_VERSION_INFO_FILE :=           fpga/pcores/system_controller_v1_00_a/hdl/vhdl/user_logic.vhd

FPGA_MCS_VERSION :=                     $(shell grep 'FPGA_VERSION_NUM.\+\:\=' $(FPGA_MCS_VERSION_INFO_FILE) |              \
                                                sed -e 's/.\+\"\(....\)\(....\)\".\+/\1.\2/' |                              \
                                                awk -F[\.] --non-decimal-data '{print ("0x"$$1)+0 "." ("0x"$$2)+0}')

.PHONY : fpga_mcs
fpga_mcs : $(FPGA_MCS_FILE) $(FPGA_MCS_VERSION_FILE) $(FPGA_MCS_MD5_FILE)

$(FPGA_MCS_FILE) : $(DOWNLOAD_BIT_FILE)
	@echo "****************************************"
	@echo "* Building FPGA (+bootloader) MCS File *"
	@echo "****************************************"
	promgen -spi -s 2048 -w -c FF -p mcs -u 0 $(DOWNLOAD_BIT_FILE) -o $@
	@rm *.prm
	@echo ""

$(FPGA_MCS_VERSION_FILE) : $(FPGA_MCS_FILE) $(FPGA_MCS_VERSION_INFO_FILE)
	@echo "**********************************"
	@echo "* Building FPGA MCS Version File *"
	@echo "**********************************"
	echo $(FPGA_MCS_VERSION) > $@
	@echo ""

$(FPGA_MCS_MD5_FILE) : $(FPGA_MCS_FILE) $(FPGA_MCS_VERSION_FILE)
	@echo "******************************"
	@echo "* Building FPGA MCS MD5 File *"
	@echo "******************************"
	md5sum $^ > $@
	@echo ""




###########################
# APP BIN IMAGE GENERATOR
###########################

APP_BIN_IMAGE_GENERATOR :=              binimage.exe

APP_BIN_IMAGE_GENERATOR_SOURCES :=      \
    firmware/binimage.c                 \
    firmware/common/crc32.c

APP_BIN_IMAGE_GENERATOR_INCLUDE_PATHS :=\
    firmware/

APP_BIN_IMAGE_GENERATOR_INCLUDES :=     $(APP_BIN_IMAGE_GENERATOR_INCLUDE_PATHS:%=-I%)

$(APP_BIN_IMAGE_GENERATOR) : $(APP_BIN_IMAGE_GENERATOR_SOURCES)
	@echo "************************************"
	@echo "* Building App Bin Image Generator *"
	@echo "************************************"
	gcc -Wall $(APP_BIN_IMAGE_GENERATOR_INCLUDES) $(APP_BIN_IMAGE_GENERATOR_SOURCES) -o $@ -lws2_32
	@echo ""




#################
# APP DUMP FILE
#################

APP_ELF_FILE :=                         firmware/app.elf
APP_ELF_VERSION_FILE :=                 $(APP_ELF_FILE).ver
APP_DUMP_FILE :=                        firmware/app.dmp
APP_OBJDUMP :=                          mb-objdump

# The buggy EDK 10.1 linker occasionally refuses to adjust "brlid" target
# addresses after removing preceding unneeded "imm" instructions.
# Fortunately, we can determine if the ELF file has any of these unadjusted
# branch targets by checking the dump file.  Properly adjusted branch
# targets are followed by a commented absolute address and the name of the
# symbol located at that address.  Unadjusted branch targets do not contain
# this information.
#
# example unadjusted branch:
#   9000903c:	b9f47d90 	brlid	r15, 32144
# example adjusted branch:
#   90000818:	b9f40680 	brlid	r15, 1664	// 90000e98 <addParamToRspPkt>
#
# Unfortunately this test doesn't work for later versions of the linker.
# Dump files generated from ELF files produced by the EDK 12.1 linker often
# contain adjusted "brlid" instructions without the absolute address and
# symbol name.

$(APP_DUMP_FILE) : $(APP_ELF_FILE)
	@echo "*************************"
	@echo "* Checking App ELF File *"
	@echo "*************************"
	$(APP_OBJDUMP) -S $(APP_ELF_FILE) > $@
	@echo Checking for bad branch targets...;\
	grep brlid.*[0-9]$$ $@;\
	if [ $$? -eq 0 ];\
	then\
	  echo "ERROR: Bad branch targets in ELF File! Moving it to \"$(APP_ELF_FILE).bad\".";\
	  mv $(APP_ELF_FILE) $(APP_ELF_FILE).bad;\
	  exit 1;\
	else\
	  echo No bad branch targets detected.;\
	fi
	@echo ""



################
# APP BIN FILE
################

APP_BIN_FILE :=                         herbert_scb.bin
APP_BIN_ELF_FILE :=                     $(APP_BIN_FILE).elf
APP_BIN_VERSION_FILE :=                 $(APP_BIN_FILE).ver
APP_BIN_MD5_FILE :=                     $(APP_BIN_FILE).md5
APP_BIN_VERSION :=                      $(shell cat $(APP_ELF_VERSION_FILE))

.PHONY : app_bin
app_bin : $(APP_BIN_FILE) $(APP_BIN_VERSION_FILE) $(APP_BIN_MD5_FILE)

$(APP_BIN_FILE) : $(APP_BIN_IMAGE_GENERATOR) $(APP_ELF_FILE) $(APP_DUMP_FILE)
	@echo "*************************"
	@echo "* Building App Bin File *"
	@echo "*************************"
	./$(APP_BIN_IMAGE_GENERATOR) $(APP_ELF_FILE) $@
	@cp $(APP_ELF_FILE) $(APP_BIN_ELF_FILE)
	@echo ""

$(APP_BIN_VERSION_FILE) : $(APP_BIN_FILE)
	@echo "*********************************"
	@echo "* Building App Bin Version File *"
	@echo "*********************************"
	@echo $(APP_BIN_VERSION) > $@
	@echo ""

$(APP_BIN_MD5_FILE) : $(APP_BIN_FILE) $(APP_BIN_VERSION_FILE)
	@echo "*****************************"
	@echo "* Building App Bin MD5 File *"
	@echo "*****************************"
	md5sum $^ > $@
	@echo ""




####################
# All, Clean, misc
####################

.PHONY : all
all : app_bin fpga_mcs

.PHONY : clean
clean :
	@rm -f $(DOWNLOAD_BIT_FILE)
	@rm -f $(FPGA_MCS_FILE)
	@rm -f $(FPGA_MCS_VERSION_FILE)
	@rm -f $(FPGA_MCS_MD5_FILE)
	@rm -f $(APP_BIN_IMAGE_GENERATOR)
	@rm -f $(APP_DUMP_FILE)
	@rm -f $(APP_BIN_FILE)
	@rm -f $(APP_BIN_VERSION_FILE)
	@rm -f $(APP_BIN_MD5_FILE)
	@rm -f $(APP_BIN_ELF_FILE)




########
# Help
########

.PHONY : help
help :
	@echo ""
	@echo "Make Options:"
	@echo "------------"
	@echo ""
	@echo "make all                 - Builds 'app_bin' and 'fpga_mcs'"
	@echo "make clean               - Removes all built files"
	@echo "make app_bin             - Builds '$(APP_BIN_FILE)', '$(APP_BIN_VERSION_FILE)', '$(APP_BIN_MD5_FILE)', and '$(APP_BIN_ELF_FILE)'"
	@echo "make fpga_mcs            - Builds '$(FPGA_MCS_FILE)', '$(FPGA_MCS_VERSION_FILE)', and '$(FPGA_MCS_MD5_FILE)'"
	@echo ""

