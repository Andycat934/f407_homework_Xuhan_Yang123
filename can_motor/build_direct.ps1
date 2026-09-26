# 直接调用 arm-none-eabi-gcc 编译 + 链接本工程（不依赖 cmake，避免 cmake configure 卡住）
# 用法：在 can_motor 目录下执行   powershell -ExecutionPolicy Bypass -File .\build_direct.ps1
# 产物：build\Debug\can_motor.elf / .hex / .bin
$env:PATH = "D:\arm-gnu-toolchain-13.3.rel1-mingw-w64-i686-arm-none-eabi\bin;$env:PATH"

$root = $PSScriptRoot
if (-not $root) { $root = (Get-Location).Path }
$out  = Join-Path $root 'build\Debug'
New-Item -ItemType Directory -Force -Path $out | Out-Null

$incs = @(
    "-I$root\Core\Inc",
    "-I$root\Drivers\STM32F4xx_HAL_Driver\Inc",
    "-I$root\Drivers\STM32F4xx_HAL_Driver\Inc\Legacy",
    "-I$root\Drivers\CMSIS\Device\ST\STM32F4xx\Include",
    "-I$root\Drivers\CMSIS\Include"
)

$cflags = @(
    '-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi=hard',
    '-O0','-g3','-Wall','-fdata-sections','-ffunction-sections',
    '-DUSE_HAL_DRIVER','-DSTM32F407xx','-DDEBUG'
)

$appSrc = @('main.c','stm32f4xx_it.c','stm32f4xx_hal_msp.c','sysmem.c','syscalls.c','system_stm32f4xx.c')
$halSrc = @('stm32f4xx_hal.c','stm32f4xx_hal_can.c','stm32f4xx_hal_cortex.c','stm32f4xx_hal_dma.c',
            'stm32f4xx_hal_dma_ex.c','stm32f4xx_hal_exti.c','stm32f4xx_hal_flash.c','stm32f4xx_hal_flash_ex.c',
            'stm32f4xx_hal_flash_ramfunc.c','stm32f4xx_hal_gpio.c','stm32f4xx_hal_pwr.c','stm32f4xx_hal_pwr_ex.c',
            'stm32f4xx_hal_rcc.c','stm32f4xx_hal_rcc_ex.c')

$objs = New-Object System.Collections.ArrayList
$fails = 0
$warncount = 0

function Compile-One($src, $obj, $tag) {
    Write-Host "CC  $tag"
    $script:lastlog = & arm-none-eabi-gcc @cflags @incs -c $src -o $obj 2>&1
    if ($LASTEXITCODE -ne 0) { $script:fails++; $script:lastlog | ForEach-Object { Write-Host "    $_" } }
    elseif ($script:lastlog) { $script:warncount++; $script:lastlog | ForEach-Object { Write-Host "    $_" } }
}

foreach ($f in $appSrc) {
    Compile-One (Join-Path "$root\Core\Src" $f) (Join-Path $out ($f -replace '\.c$', '.o')) "Core/$f"
    [void]$objs.Add((Join-Path $out ($f -replace '\.c$', '.o')))
}
foreach ($f in $halSrc) {
    Compile-One (Join-Path "$root\Drivers\STM32F4xx_HAL_Driver\Src" $f) (Join-Path $out ($f -replace '\.c$', '.o')) "HAL/$f"
    [void]$objs.Add((Join-Path $out ($f -replace '\.c$', '.o')))
}

$startupObj = Join-Path $out 'startup_stm32f407xx.o'
Write-Host "AS  startup_stm32f407xx.s"
& arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -x assembler-with-cpp -c (Join-Path $root 'startup_stm32f407xx.s') -o $startupObj 2>&1 | ForEach-Object { Write-Host "    $_" }
if ($LASTEXITCODE -ne 0) { $fails++ }
[void]$objs.Add($startupObj)

Write-Host "LD  can_motor.elf"
$ldflags = @(
    '-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi=hard',
    "-T", (Join-Path $root 'STM32F407XX_FLASH.ld'),
    '--specs=nano.specs', "-Wl,-Map=$out\can_motor.map", '-Wl,--gc-sections', '-Wl,--print-memory-usage'
)
& arm-none-eabi-gcc @ldflags @objs -o "$out\can_motor.elf" -lm 2>&1 | ForEach-Object { Write-Host "    $_" }
if ($LASTEXITCODE -ne 0) { $fails++; Write-Host "LINK FAILED" }
else {
    Write-Host "OBJCOPY hex/bin"
    & arm-none-eabi-objcopy -O ihex   "$out\can_motor.elf" "$out\can_motor.hex"
    & arm-none-eabi-objcopy -O binary "$out\can_motor.elf" "$out\can_motor.bin"
    & arm-none-eabi-size "$out\can_motor.elf"
}

Write-Host ""
Write-Host "COMPILE ERRORS: $fails   (files with warnings: $warncount)"
Write-Host "ELF: $out\can_motor.elf"
exit $fails
