Import("env")

env.Replace(
    UPLOADER="stcgal",
    UPLOADCMD=(
        "$UPLOADER -P $CUSTOM_STCGAL_PROTOCOL -p $UPLOAD_PORT "
        "-b $CUSTOM_STCGAL_BAUD --trim $CUSTOM_STCGAL_TRIM $SOURCE"
    ),
)
