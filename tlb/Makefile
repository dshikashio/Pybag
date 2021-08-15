OUTPUT_DIR = out
BASE_NAME  = DbgEng
INPUT_HEADER = sdk\dbgeng.h

all: tlb

tlb: $(OUTPUT_DIR)\$(BASE_NAME).tlb

idl: $(OUTPUT_DIR)\$(BASE_NAME).idl

.IGNORE:
clean:
			del /f /q $(OUTPUT_DIR)\*
			rmdir $(OUTPUT_DIR)
!CMDSWITCHES

$(OUTPUT_DIR):
			mkdir $(OUTPUT_DIR)

$(OUTPUT_DIR)\$(BASE_NAME).tlb: $(OUTPUT_DIR)\$(BASE_NAME).idl
			midl /tlb $@ $(OUTPUT_DIR)\$(BASE_NAME).idl
			
$(OUTPUT_DIR)\$(BASE_NAME).idl: header.idl $(OUTPUT_DIR)\body.idl footer.idl
			copy header.idl+$(OUTPUT_DIR)\body.idl+footer.idl $@

$(OUTPUT_DIR)\body.idl: $(OUTPUT_DIR) $(INPUT_HEADER) hpp_to_idl.pl
			perl hpp_to_idl.pl $(INPUT_HEADER) $@
