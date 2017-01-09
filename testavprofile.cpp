/*

*/
#include <iostream>

#include "testavprofile.hpp"

#include "optionparser.h"       /* lean mean C++ options parser */

enum  optionIndex { UNKNOWN, HELP, PLUS };
const option::Descriptor usage[] =
{
    {UNKNOWN, 0,"" , ""    , option::Arg::None, "USAGE: example [options]\n\n"
                                                "Options:" },
    {HELP,    0,"" , "help", option::Arg::None, "  --help  \tPrint usage and exit." },
    {PLUS,    0,"p", "plus", option::Arg::None, "  --plus, -p  \tIncrement count." },
    {UNKNOWN, 0,"" ,  ""   , option::Arg::None, "\nExamples:\n"
                                                "  example --unknown -- --this_is_no_option\n"
                                                "  example -unk --plus -ppp file1 file2\n" },
    {0,0,0,0,0,0}
};

/* ________________________________________________________________________ */

class avStream
{
private:
    AVStream *st;
    AVCodecContext *dec_ctx;
};

class avFile
{

private:
    AVFormatContext *fmt_ctx;

    InputStream *streams;
    int       nb_streams;

public:
    avFile()
    {
        fmt_ctx = NULL;
    }

    ~avFile()
    {
        close();
    }

    open( const char *filename )
    {
        int err, i;
        int orig_nb_streams;
        AVDictionaryEntry *t;
        AVDictionary **opts;
        int scan_all_pmts_set = 0;

        err = avformat_open_input( &fmt_ctx, filename, NULL, NULL );
        if ( err < 0 ) {
            print_error(filename, err);
            return err;
        }


        /* fill the streams in the format context */
        opts = setup_find_stream_info_opts(fmt_ctx, codec_opts);
        orig_nb_streams = fmt_ctx->nb_streams;

        err = avformat_find_stream_info(fmt_ctx, opts);

        for (i = 0; i < orig_nb_streams; i++)
            av_dict_free(&opts[i]);

        av_freep(&opts);

        if (err < 0) {
            print_error(filename, err);
            return err;
        }

        av_dump_format(fmt_ctx, 0, filename, 0);

        ifile->streams = av_mallocz_array(fmt_ctx->nb_streams, sizeof(*ifile->streams));
        if (!ifile->streams)
            exit(1);
        ifile->nb_streams = fmt_ctx->nb_streams;

        /* bind a decoder to each input stream */
        for (i = 0; i < fmt_ctx->nb_streams; i++)
        {
            InputStream *ist = &ifile->streams[i];
            AVStream *stream = fmt_ctx->streams[i];
            AVCodec *codec;

            ist->st = stream;

            if (stream->codecpar->codec_id == AV_CODEC_ID_PROBE)
            {
                av_log(NULL, AV_LOG_WARNING,
                       "Failed to probe codec for input stream %d\n",
                        stream->index);
                continue;
            }

            codec = avcodec_find_decoder(stream->codecpar->codec_id);
            if (!codec) {
                av_log(NULL, AV_LOG_WARNING,
                        "Unsupported codec with id %d for input stream %d\n",
                        stream->codecpar->codec_id, stream->index);
                continue;
            }

            {
                AVDictionary *opts = filter_codec_opts(codec_opts, stream->codecpar->codec_id,
                                                       fmt_ctx, stream, codec);

                ist->dec_ctx = avcodec_alloc_context3(codec);
                if (!ist->dec_ctx)
                    exit(1);

                err = avcodec_parameters_to_context(ist->dec_ctx, stream->codecpar);
                if (err < 0)
                    exit(1);

                av_codec_set_pkt_timebase(ist->dec_ctx, stream->time_base);
                ist->dec_ctx->framerate = stream->avg_frame_rate;

                if (avcodec_open2(ist->dec_ctx, codec, &opts) < 0) {
                    av_log(NULL, AV_LOG_WARNING, "Could not open codec for input stream %d\n",
                           stream->index);
                    exit(1);
                }

                if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
                    av_log(NULL, AV_LOG_ERROR, "Option %s for input stream %d not found\n",
                           t->key, stream->index);
                    return AVERROR_OPTION_NOT_FOUND;
                }
            }
        }

        ifile->fmt_ctx = fmt_ctx;
        return 0;
    }

    void close(void)
    {
        if ( fmt_ctx != NULL )
        {
            avformat_close_input( &fmt_ctx );
            fmt_ctx = NULL;
        }
    }

};

/* ________________________________________________________________________ */

int testFile( char *file )
{
	int result = 0;

	return result;
}

static int writeCallBack(void *opaque, uint8_t *buf, int buf_size)
{
    printf("%.*s", buf_size, buf);
    return 0;
}

AVIOContext *newAVIOContext(void)
{
	uint8_t *buffer;
	AVIOContext *result = NULL;

    buffer = (uint8_t *)av_malloc(AVP_BUFFSIZE);
    if (buffer != NULL)
    {
    	result = avio_alloc_context( buffer, AVP_BUFFSIZE, 1, NULL, NULL,
    		                        writeCallBack, NULL);
	}
	return result;
}

void disposeAVIOContext(AVIOContext *ioContext)
{
    avio_flush( ioContext );

    av_freep( (void *)ioContext->buffer );
    av_freep( (void *)ioContext->av_class );
}

/* initialize and clean up the ffmpeg libav* libraries */
class libavInstance
{
public:
    libavInstance()
    {
        av_register_all();
        avdevice_register_all();

        avformat_network_init();
    }

    ~libavInstance()
    {
        avformat_network_deinit();
    }
};

/* ________________________________________________________________________ */

int main(int argc, char **argv)
{
    int exitCode = -1;

    libavInstance   libav;

    AVIOContext *ioContext = NULL;

    argc -= (argc > 0); argv += (argc > 0); // skip program name argv[0] if present

    option::Stats   stats(usage, argc, argv);
    option::Option  options[stats.options_max];
    option::Option  buffer[stats.buffer_max];
    option::Parser  parse(usage, argc, argv, options, buffer);

    if ( parse.error() )
    {
        return 1;
    }

    if ( options[HELP] || argc == 0 )
    {
        option::printUsage(std::cout, usage);
        return 0;
    }

    std::cout << "\n--plus count: " << options[PLUS].count();

    option::Option *opt = options[UNKNOWN];
    while ( opt != NULL )
    {
        std::cout << "\nUnknown option: " << opt->name;
        opt = opt->next();
    }

    for (int i = 0; i < parse.nonOptionsCount(); ++i)
    {
        std::cout << "\nNon-option #" << i << ": " << parse.nonOption(i);
    }

    std::cout << "\n";

    /* ________________________________________________________________________ */

	ioContext = newAVIOContext();

    if (ioContext != NULL)
    {
	    exitCode = testFile( argv[1] );

	    disposeAVIOContext( ioContext );
    }

    return exitCode;
}
