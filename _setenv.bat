SET ASMDIR=bootldr
SET HLLMDIR=vkern\Release
SET LOGSDIRNAME=logs

@rem _vm_config should define:
@rem
@rem SET VMDIR=S:\DEV\VM
@rem SET VBOXDIR=S:\Programs\VirtualBox
@rem SET VMNAME=osName
@rem SET VMSTGCTL=IDE
@rem SET VMHD=hd_osName.vdi
@rem SET VMPIPE=\\.\pipe\vboxCOM1
CALL _vm_config


SET LDR1ASM=bl_boot.asm
SET LDR1BIN=bl_boot.bin
SET LDR2ASM=bl_loader.asm
SET LDR2BIN=bl_loader.bin
SET LDR2HLLM=vkern.exe
SET LDR2MIXED=bl_ldrmix.bin


SET LDR1_SOURCE="%ASMDIR%\%LDR1ASM%"
SET LDR1_IMAGE="%ASMDIR%\%LDR1BIN%"

SET LDR2_SOURCE="%ASMDIR%\%LDR2ASM%"
SET LDR2_IMAGE="%ASMDIR%\%LDR2BIN%"
SET LDR2_HLLMODULE="%HLLMDIR%\%LDR2HLLM%"
SET LDR2_MIXED="%ASMDIR%\%LDR2MIXED%"

SET PIPER="..\piper.exe" -pipe:%VMPIPE% -image:"..\%ASMDIR%\%LDR2MIXED%" -addr:0x0800 -entry:0x0000

SET VBM="%VBOXDIR%\VBoxManage.exe"
SET VMHDIMAGE="%VMDIR%\%VMNAME%\%VMHD%"
SET VMHDBACKUP="%VMDIR%\%VMNAME%\%VMHD%.bak"


