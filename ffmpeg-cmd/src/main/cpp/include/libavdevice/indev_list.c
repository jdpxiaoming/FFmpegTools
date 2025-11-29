#include "alldevices.c"

static const FFInputFormat * const indev_list[] = {
    &ff_fbdev_demuxer,
    &ff_lavfi_demuxer,
    &ff_v4l2_demuxer,
    NULL };
