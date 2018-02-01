cd driver_functions_ChannelExtraction

echo "llq"

make -f channel_extraction_wiener.make
make install -f channel_extraction_wiener.make
