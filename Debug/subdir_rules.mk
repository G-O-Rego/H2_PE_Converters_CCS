################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"E:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 --fp_mode=relaxed --include_path="E:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --include_path="E:/ti/c2000/C2000Ware_5_02_00_00/device_support/f28002x/headers/include" --include_path="E:/ti/c2000/C2000Ware_5_02_00_00/device_support/f28002x/common/include" --advice:performance=all -g --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"E:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --idiv_support=idiv0 --tmu_support=tmu0 --fp_mode=relaxed --include_path="E:/ti/ccs1271/ccs/tools/compiler/ti-cgt-c2000_22.6.1.LTS/include" --include_path="E:/ti/c2000/C2000Ware_5_02_00_00/device_support/f28002x/headers/include" --include_path="E:/ti/c2000/C2000Ware_5_02_00_00/device_support/f28002x/common/include" --advice:performance=all -g --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


