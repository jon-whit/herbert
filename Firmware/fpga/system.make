#################################################################
# Makefile generated by Xilinx Platform Studio 
# Project:C:\Github\herbert\Firmware\fpga\system.xmp
#
# WARNING : This file will be re-generated every time a command
# to run a make target is invoked. So, any changes made to this  
# file manually, will be lost when make is invoked next. 
#################################################################

# Name of the Microprocessor system
# The hardware specification of the system is in file :
# C:\Github\herbert\Firmware\fpga\system.mhs
# The software specification of the system is in file :
# C:\Github\herbert\Firmware\fpga\system.mss

include system_incl.make

#################################################################
# PHONY TARGETS
#################################################################
.PHONY: dummy
.PHONY: netlistclean
.PHONY: bitsclean
.PHONY: simclean
.PHONY: vpclean

#################################################################
# EXTERNAL TARGETS
#################################################################
all:
	@echo "Makefile to build a Microprocessor system :"
	@echo "Run make with any of the following targets"
	@echo " "
	@echo "  netlist  : Generates the netlist for the given MHS "
	@echo "  bits     : Runs Implementation tools to generate the bitstream"
	@echo " "
	@echo "  libs     : Configures the sw libraries for this system"
	@echo "  program  : Compiles the program sources for all the processor instances"
	@echo " "
	@echo "  init_bram: Initializes bitstream with BRAM data"
	@echo "  ace      : Generate ace file from bitstream and elf"
	@echo "  download : Downloads the bitstream onto the board"
	@echo " "
	@echo "  sim      : Generates HDL simulation models and runs simulator for chosen simulation mode"
	@echo "  simmodel : Generates HDL simulation models for chosen simulation mode"
	@echo "  behavioral_model : Generates behavioral HDL models with BRAM initialization"
	@echo "  structural_model : Generates structural simulation HDL models with BRAM initialization"
	@echo "  timing   : Generates timing simulation HDL models with BRAM initialization"
	@echo "  vp       : Generates virtual platform model"
	@echo " "
	@echo "  netlistclean: Deletes netlist"
	@echo "  bitsclean: Deletes bit, ncd, bmm files"
	@echo "  hwclean  : Deletes implementation dir"
	@echo "  libsclean: Deletes sw libraries"
	@echo "  programclean: Deletes compiled ELF files"
	@echo "  swclean  : Deletes sw libraries and ELF files"
	@echo "  simclean : Deletes simulation dir"
	@echo "  vpclean  : Deletes virtualplatform dir"
	@echo "  clean    : Deletes all generated files/directories"
	@echo " "
	@echo "  make <target> : (Default)"
	@echo "      Creates a Microprocessor system using default initializations"
	@echo "      specified for each processor in MSS file"


bits: $(SYSTEM_BIT)

ace: $(SYSTEM_ACE)

netlist: $(POSTSYN_NETLIST)

libs: $(LIBRARIES)

program: $(ALL_USER_ELF_FILES)

download: $(DOWNLOAD_BIT) dummy
	@echo "*********************************************"
	@echo "Downloading Bitstream onto the target board"
	@echo "*********************************************"
	impact -batch etc/download.cmd

init_bram: $(DOWNLOAD_BIT)

sim: $(DEFAULT_SIM_SCRIPT)
	cd simulation/behavioral; \
	$(SIM_CMD)  &

simmodel: $(DEFAULT_SIM_SCRIPT)

behavioral_model: $(BEHAVIORAL_SIM_SCRIPT)

structural_model: $(STRUCTURAL_SIM_SCRIPT)

vp: $(VPEXEC)

clean: hwclean libsclean programclean simclean vpclean
	rm -f _impact.cmd
	rm -f *.log

hwclean: netlistclean bitsclean
	rm -rf implementation synthesis xst hdl
	rm -rf xst.srp $(SYSTEM).srp

netlistclean:
	rm -f $(POSTSYN_NETLIST)
	rm -f platgen.log
	rm -f $(BMM_FILE)

bitsclean:
	rm -f $(SYSTEM_BIT)
	rm -f implementation/$(SYSTEM).ncd
	rm -f implementation/$(SYSTEM)_bd.bmm 
	rm -f implementation/$(SYSTEM)_map.ncd 
	rm -f __xps/$(SYSTEM)_routed

simclean: 
	rm -rf simulation/behavioral
	rm -f simgen.log

swclean: libsclean programclean
	@echo ""

libsclean: $(LIBSCLEAN_TARGETS)
	rm -f libgen.log

programclean: $(PROGRAMCLEAN_TARGETS)

vpclean:
	rm -rf virtualplatform
	rm -f vpgen.log

#################################################################
# SOFTWARE PLATFORM FLOW
#################################################################


$(LIBRARIES): $(MHSFILE) $(MSSFILE) __xps/libgen.opt
	@echo "*********************************************"
	@echo "Creating software libraries..."
	@echo "*********************************************"
	libgen $(LIBGEN_OPTIONS) $(MSSFILE)


microblaze_0_libsclean:
	rm -rf microblaze_0/

#################################################################
# SOFTWARE APPLICATION BOOTLOADER
#################################################################

bootloader_program: $(BOOTLOADER_OUTPUT) 

$(BOOTLOADER_OUTPUT) : $(BOOTLOADER_SOURCES) $(BOOTLOADER_HEADERS) $(BOOTLOADER_LINKER_SCRIPT) \
                    $(LIBRARIES) __xps/bootloader_compiler.opt
	@mkdir -p $(BOOTLOADER_OUTPUT_DIR) 
	$(BOOTLOADER_CC) $(BOOTLOADER_CC_OPT) $(BOOTLOADER_SOURCES) -o $(BOOTLOADER_OUTPUT) \
	$(BOOTLOADER_OTHER_CC_FLAGS) $(BOOTLOADER_INCLUDES) $(BOOTLOADER_LIBPATH) \
	$(BOOTLOADER_CFLAGS) $(BOOTLOADER_LFLAGS) 
	$(BOOTLOADER_CC_SIZE) $(BOOTLOADER_OUTPUT) 
	@echo ""

bootloader_programclean:
	rm -f $(BOOTLOADER_OUTPUT) 

#################################################################
# SOFTWARE APPLICATION APPLICATION
#################################################################

application_program: $(APPLICATION_OUTPUT) 

$(APPLICATION_OUTPUT) : $(APPLICATION_SOURCES) $(APPLICATION_HEADERS) $(APPLICATION_LINKER_SCRIPT) \
                    $(LIBRARIES) __xps/application_compiler.opt
	@mkdir -p $(APPLICATION_OUTPUT_DIR) 
	$(APPLICATION_CC) $(APPLICATION_CC_OPT) $(APPLICATION_SOURCES) -o $(APPLICATION_OUTPUT) \
	$(APPLICATION_OTHER_CC_FLAGS) $(APPLICATION_INCLUDES) $(APPLICATION_LIBPATH) \
	$(APPLICATION_CFLAGS) $(APPLICATION_LFLAGS) 
	$(APPLICATION_CC_SIZE) $(APPLICATION_OUTPUT) 
	@echo ""

application_programclean:
	rm -f $(APPLICATION_OUTPUT) 

#################################################################
# BOOTLOOP ELF FILES
#################################################################



$(MICROBLAZE_0_BOOTLOOP): $(MICROBLAZE_BOOTLOOP)
	@mkdir -p $(BOOTLOOP_DIR)
	cp -f $(MICROBLAZE_BOOTLOOP) $(MICROBLAZE_0_BOOTLOOP)

#################################################################
# HARDWARE IMPLEMENTATION FLOW
#################################################################


$(BMM_FILE) \
$(WRAPPER_NGC_FILES): $(MHSFILE) __xps/platgen.opt \
                      $(CORE_STATE_DEVELOPMENT_FILES)
	@echo "****************************************************"
	@echo "Creating system netlist for hardware specification.."
	@echo "****************************************************"
	platgen $(PLATGEN_OPTIONS) $(MHSFILE)

$(POSTSYN_NETLIST): $(WRAPPER_NGC_FILES)
	@echo "Running synthesis..."
	bash -c "cd synthesis; ./synthesis.sh"

__xps/$(SYSTEM)_routed: $(FPGA_IMP_DEPENDENCY)
	@echo "*********************************************"
	@echo "Running Xilinx Implementation tools.."
	@echo "*********************************************"
	@cp -f $(UCF_FILE) implementation/$(SYSTEM).ucf
	xilperl $(NON_CYG_XILINX_EDK_DIR)/data/fpga_impl/manage_fastruntime_opt.pl $(MANAGE_FASTRT_OPTIONS)
	xflow -wd implementation -p $(DEVICE) -implement xflow.opt $(SYSTEM).ngc
	touch __xps/$(SYSTEM)_routed

$(SYSTEM_BIT): __xps/$(SYSTEM)_routed $(BITGEN_UT_FILE)
	xilperl $(NON_CYG_XILINX_EDK_DIR)/data/fpga_impl/observe_par.pl $(OBSERVE_PAR_OPTIONS) implementation/$(SYSTEM).par
	@echo "*********************************************"
	@echo "Running Bitgen.."
	@echo "*********************************************"
	@cp -f $(BITGEN_UT_FILE) implementation/bitgen.ut
	cd implementation; bitgen -w -f bitgen.ut $(SYSTEM)

$(DOWNLOAD_BIT): $(SYSTEM_BIT) $(BRAMINIT_ELF_FILES) __xps/bitinit.opt
	@cp -f implementation/$(SYSTEM)_bd.bmm .
	@echo "*********************************************"
	@echo "Initializing BRAM contents of the bitstream"
	@echo "*********************************************"
	bitinit $(MHSFILE) $(SEARCHPATHOPT) $(BRAMINIT_ELF_FILE_ARGS) \
	-bt $(SYSTEM_BIT) -o $(DOWNLOAD_BIT)
	@rm -f $(SYSTEM)_bd.bmm

$(SYSTEM_ACE):
	@echo "In order to generate ace file, you must have:-"
	@echo "- exactly one processor."
	@echo "- opb_mdm, if using microblaze."

#################################################################
# SIMULATION FLOW
#################################################################


################## BEHAVIORAL SIMULATION ##################

$(BEHAVIORAL_SIM_SCRIPT): $(MHSFILE) __xps/simgen.opt \
                          $(BRAMINIT_ELF_FILES)
	@echo "*********************************************"
	@echo "Creating behavioral simulation models..."
	@echo "*********************************************"
	simgen $(SIMGEN_OPTIONS) -m behavioral $(MHSFILE)

################## STRUCTURAL SIMULATION ##################

$(STRUCTURAL_SIM_SCRIPT): $(WRAPPER_NGC_FILES) __xps/simgen.opt \
                          $(BRAMINIT_ELF_FILES)
	@echo "*********************************************"
	@echo "Creating structural simulation models..."
	@echo "*********************************************"
	simgen $(SIMGEN_OPTIONS) -sd implementation -m structural $(MHSFILE)


################## TIMING SIMULATION ##################

$(TIMING_SIM_SCRIPT): $(SYSTEM_BIT) __xps/simgen.opt \
                      $(BRAMINIT_ELF_FILES)
	@echo "*********************************************"
	@echo "Creating timing simulation models..."
	@echo "*********************************************"
	simgen $(SIMGEN_OPTIONS) -sd implementation -m timing $(MHSFILE)

#################################################################
# VIRTUAL PLATFORM FLOW
#################################################################


$(VPEXEC): $(MHSFILE) __xps/vpgen.opt
	@echo "****************************************************"
	@echo "Creating virtual platform for hardware specification.."
	@echo "****************************************************"
	vpgen $(VPGEN_OPTIONS) $(MHSFILE)

dummy:
	@echo ""
