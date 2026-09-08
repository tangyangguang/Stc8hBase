Import("env")

protocol = env.GetProjectOption("custom_stcgal_protocol", "stc8g")
baud = env.GetProjectOption("custom_stcgal_baud", "9600")
trim = env.GetProjectOption("custom_stcgal_trim", "").strip()
eeprom_split = env.GetProjectOption("custom_stcgal_eeprom_split", "").strip()

trim_flags = ("-t %s " % trim) if trim else ""
eeprom_split_flags = (
    "-o program_eeprom_split=%s " % eeprom_split
) if eeprom_split else ""

env.Replace(
    UPLOADCMD=(
        '"$PYTHONEXE" "$UPLOADER" '
        '-P %s '
        '-p "$UPLOAD_PORT" '
        '%s'
        '%s'
        '-a '
        '-b %s '
        '$SOURCE'
    ) % (protocol, trim_flags, eeprom_split_flags, baud)
)
