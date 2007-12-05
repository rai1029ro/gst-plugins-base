/* GStreamer
 * Copyright (C) <2007> Wim Taymans <wim.taymans@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-playbin2
 *
 * <refsect2>
 * <para>
 * Playbin provides a stand-alone everything-in-one abstraction for an
 * audio and/or video player.
 * </para>
 * <para>
 * It can handle both audio and video files and features
 * <itemizedlist>
 * <listitem>
 * automatic file type recognition and based on that automatic
 * selection and usage of the right audio/video/subtitle demuxers/decoders
 * </listitem>
 * <listitem>
 * visualisations for audio files
 * </listitem>
 * <listitem>
 * subtitle support for video files
 * </listitem>
 * <listitem>
 * stream selection between different audio/subtitles streams
 * </listitem>
 * <listitem>
 * meta info (tag) extraction
 * </listitem>
 * <listitem>
 * easy access to the last video frame
 * </listitem>
 * <listitem>
 * buffering when playing streams over a network
 * </listitem>
 * <listitem>
 * volume control
 * </listitem>
 * </itemizedlist>
 * </para>
 * <title>Usage</title>
 * <para>
 * A playbin element can be created just like any other element using
 * gst_element_factory_make(). The file/URI to play should be set via the "uri"
 * property. This must be an absolute URI, relative file paths are not allowed.
 * Example URIs are file:///home/joe/movie.avi or http://www.joedoe.com/foo.ogg
 * </para>
 * <para>
 * Playbin is a #GstPipeline. It will notify the application of everything
 * that's happening (errors, end of stream, tags found, state changes, etc.)
 * by posting messages on its #GstBus. The application needs to watch the
 * bus.
 * </para>
 * <para>
 * Playback can be initiated by setting the element to PLAYING state using
 * gst_element_set_state(). Note that the state change will take place in
 * the background in a separate thread, when the function returns playback
 * is probably not happening yet and any errors might not have occured yet.
 * Applications using playbin should ideally be written to deal with things
 * completely asynchroneous.
 * </para>
 * <para>
 * When playback has finished (an EOS message has been received on the bus)
 * or an error has occured (an ERROR message has been received on the bus) or
 * the user wants to play a different track, playbin should be set back to
 * READY or NULL state, then the "uri" property should be set to the new
 * location and then playbin be set to PLAYING state again.
 * </para>
 * <para>
 * Seeking can be done using gst_element_seek_simple() or gst_element_seek()
 * on the playbin element. Again, the seek will not be executed
 * instantaneously, but will be done in a background thread. When the seek
 * call returns the seek will most likely still be in process. An application
 * may wait for the seek to finish (or fail) using gst_element_get_state() with
 * -1 as the timeout, but this will block the user interface and is not
 * recommended at all.
 * </para>
 * <para>
 * Applications may query the current position and duration of the stream
 * via gst_element_query_position() and gst_element_query_duration() and
 * setting the format passed to GST_FORMAT_TIME. If the query was successful,
 * the duration or position will have been returned in units of nanoseconds.
 * </para>
 * <title>Advanced Usage: specifying the audio and video sink</title>
 * <para>
 * By default, if no audio sink or video sink has been specified via the
 * "audio-sink" or "video-sink" property, playbin will use the autoaudiosink
 * and autovideosink elements to find the first-best available output method.
 * This should work in most cases, but is not always desirable. Often either
 * the user or application might want to specify more explicitly what to use
 * for audio and video output.
 * </para>
 * <para>
 * If the application wants more control over how audio or video should be
 * output, it may create the audio/video sink elements itself (for example
 * using gst_element_factory_make()) and provide them to playbin using the
 * "audio-sink" or "video-sink" property.
 * </para>
 * <para>
 * GNOME-based applications, for example, will usually want to create
 * gconfaudiosink and gconfvideosink elements and make playbin use those,
 * so that output happens to whatever the user has configured in the GNOME
 * Multimedia System Selector confinguration dialog.
 * </para>
 * <para>
 * The sink elements do not necessarily need to be ready-made sinks. It is
 * possible to create container elements that look like a sink to playbin,
 * but in reality contain a number of custom elements linked together. This
 * can be achieved by creating a #GstBin and putting elements in there and
 * linking them, and then creating a sink #GstGhostPad for the bin and pointing
 * it to the sink pad of the first element within the bin. This can be used
 * for a number of purposes, for example to force output to a particular
 * format or to modify or observe the data before it is output.
 * </para>
 * <para>
 * It is also possible to 'suppress' audio and/or video output by using
 * 'fakesink' elements (or capture it from there using the fakesink element's
 * "handoff" signal, which, nota bene, is fired from the streaming thread!).
 * </para>
 * <title>Retrieving Tags and Other Meta Data</title>
 * <para>
 * Most of the common meta data (artist, title, etc.) can be retrieved by
 * watching for TAG messages on the pipeline's bus (see above).
 * </para>
 * <para>
 * Other more specific meta information like width/height/framerate of video
 * streams or samplerate/number of channels of audio streams can be obtained
 * using the "stream-info" property, which will return a GList of stream info
 * objects, one for each stream. These are opaque objects that can only be
 * accessed via the standard GObject property interface, ie. g_object_get().
 * Each stream info object has the following properties:
 * <itemizedlist>
 * <listitem>"object" (GstObject) (the decoder source pad usually)</listitem>
 * <listitem>"type" (enum) (if this is an audio/video/subtitle stream)</listitem>
 * <listitem>"decoder" (string) (name of decoder used to decode this stream)</listitem>
 * <listitem>"mute" (boolean) (to mute or unmute this stream)</listitem>
 * <listitem>"caps" (GstCaps) (caps of the decoded stream)</listitem>
 * <listitem>"language-code" (string) (ISO-639 language code for this stream, mostly used for audio/subtitle streams)</listitem>
 * <listitem>"codec" (string) (format this stream was encoded in)</listitem>
 * </itemizedlist>
 * Stream information from the stream-info properties is best queried once
 * playbin has changed into PAUSED or PLAYING state (which can be detected
 * via a state-changed message on the bus where old_state=READY and
 * new_state=PAUSED), since before that the list might not be complete yet or
 * not contain all available information (like language-codes).
 * </para>
 * <title>Buffering</title>
 * <para>
 * Playbin handles buffering automatically for the most part, but applications
 * need to handle parts of the buffering process as well. Whenever playbin is
 * buffering, it will post BUFFERING messages on the bus with a percentage
 * value that shows the progress of the buffering process. Applications need
 * to set playbin to PLAYING or PAUSED state in response to these messages.
 * They may also want to convey the buffering progress to the user in some
 * way. Here is how to extract the percentage information from the message
 * (requires GStreamer >= 0.10.11):
 * </para>
 * <para>
 * <programlisting>
 * switch (GST_MESSAGE_TYPE (msg)) {
 *   case GST_MESSAGE_BUFFERING: {
 *     gint percent = 0;
 *     gst_message_parse_buffering (msg, &amp;percent);
 *     g_print ("Buffering (%%u percent done)", percent);
 *     break;
 *   }
 *   ...
 * }
 * </programlisting>
 * Note that applications should keep/set the pipeline in the PAUSED state when
 * a BUFFERING message is received with a buffer percent value < 100 and set
 * the pipeline back to PLAYING state when a BUFFERING message with a value
 * of 100 percent is received (if PLAYING is the desired state, that is).
 * </para>
 * <title>Embedding the video window in your application</title>
 * <para>
 * By default, playbin (or rather the video sinks used) will create their own
 * window. Applications will usually want to force output to a window of their
 * own, however. This can be done using the GstXOverlay interface, which most
 * video sinks implement. See the documentation there for more details.
 * </para>
 * <title>Specifying which CD/DVD device to use</title>
 * <para>
 * The device to use for CDs/DVDs needs to be set on the source element
 * playbin creates before it is opened. The only way to do this at the moment
 * is to connect to playbin's "notify::source" signal, which will be emitted
 * by playbin when it has created the source element for a particular URI.
 * In the signal callback you can check if the source element has a "device"
 * property and set it appropriately. In future ways might be added to specify
 * the device as part of the URI, but at the time of writing this is not
 * possible yet.
 * </para>
 * <title>Examples</title>
 * <para>
 * Here is a simple pipeline to play back a video or audio file:
 * <programlisting>
 * gst-launch -v playbin uri=file:///path/to/somefile.avi
 * </programlisting>
 * This will play back the given AVI video file, given that the video and
 * audio decoders required to decode the content are installed. Since no
 * special audio sink or video sink is supplied (not possible via gst-launch),
 * playbin will try to find a suitable audio and video sink automatically
 * using the autoaudiosink and autovideosink elements.
 * </para>
 * <para>
 * Here is a another pipeline to play track 4 of an audio CD:
 * <programlisting>
 * gst-launch -v playbin uri=cdda://4
 * </programlisting>
 * This will play back track 4 on an audio CD in your disc drive (assuming
 * the drive is detected automatically by the plugin).
 * </para>
 * <para>
 * Here is a another pipeline to play title 1 of a DVD:
 * <programlisting>
 * gst-launch -v playbin uri=dvd://1
 * </programlisting>
 * This will play back title 1 of a DVD in your disc drive (assuming
 * the drive is detected automatically by the plugin).
 * </para>
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <gst/gst.h>

#include <gst/gst-i18n-plugin.h>
#include <gst/pbutils/pbutils.h>

#include "gstplay-enum.h"
#include "gstplaysink.h"
#include "gstfactorylists.h"
#include "gststreaminfo.h"
#include "gststreamselector.h"

GST_DEBUG_CATEGORY_STATIC (gst_play_bin_debug);
#define GST_CAT_DEFAULT gst_play_bin_debug

#define GST_TYPE_PLAY_BIN               (gst_play_bin_get_type())
#define GST_PLAY_BIN(obj)               (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_PLAY_BIN,GstPlayBin))
#define GST_PLAY_BIN_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_PLAY_BIN,GstPlayBinClass))
#define GST_IS_PLAY_BIN(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_PLAY_BIN))
#define GST_IS_PLAY_BIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_PLAY_BIN))

#define VOLUME_MAX_DOUBLE 10.0
#define CONNECTION_SPEED_DEFAULT 0

typedef struct _GstPlayBin GstPlayBin;
typedef struct _GstPlayBinClass GstPlayBinClass;
typedef struct _GstSourceGroup GstSourceGroup;
typedef struct _GstSourceSelect GstSourceSelect;

/* has the info for a selector and provides the link to the sink */
struct _GstSourceSelect
{
  const gchar *media;           /* the media type of the selector */
  GstPlaySinkType type;         /* the sink pad type of the selector */
  GstPlaySinkMode mode;         /* the sink pad mode of the selector */

  GstElement *selector;         /* the selector */
  gint current;                 /* the currently selected stream */
  GstPad *srcpad;               /* the source pad of the selector */
  GstPad *sinkpad;              /* the sinkpad of the sink when the selector is linked */
};

/* a structure to hold the objects for decoding a uri and the subtitle uri
 */
struct _GstSourceGroup
{
  GstPlayBin *playbin;

  gboolean valid;               /* the group has valid info to start playback */

  /* properties */
  gchar *uri;
  gchar *suburi;
  GValueArray *streaminfo;
  GstElement *source;
  gchar *subencoding;           /* encoding to propagate to the subtitle elements */

  /* uridecodebins for uri and subtitle uri */
  GstElement *uridecodebin;
  GstElement *suburidecodebin;

  /* selectors for different streams */
  GstSourceSelect selector[GST_PLAY_SINK_TYPE_LAST];
};

struct _GstPlayBin
{
  GstPipeline parent;

  /* the groups, we use a double buffer to switch between current and next */
  GstSourceGroup groups[2];     /* array with group info */
  GstSourceGroup *curr_group;   /* pointer to the currently playing group */
  GstSourceGroup *next_group;   /* pointer to the next group */

  /* properties */
  guint connection_speed;       /* connection speed in bits/sec (0 = unknown) */

  /* our play sink */
  GstPlaySink *playsink;

  GValueArray *elements;        /* factories we can use for selecting elements */
};

struct _GstPlayBinClass
{
  GstPipelineClass parent_class;

  void (*about_to_finish) (void);
};

/* props */
enum
{
  PROP_0,
  PROP_URI,
  PROP_SUBURI,
  PROP_NEXT_URI,
  PROP_NEXT_SUBURI,
  PROP_STREAMINFO,
  PROP_SOURCE,
  PROP_CURRENT_VIDEO,
  PROP_CURRENT_AUDIO,
  PROP_CURRENT_TEXT,
  PROP_SUBTITLE_ENCODING,
  PROP_AUDIO_SINK,
  PROP_VIDEO_SINK,
  PROP_VIS_PLUGIN,
  PROP_VOLUME,
  PROP_FRAME,
  PROP_FONT_DESC,
  PROP_CONNECTION_SPEED
};

/* signals */
enum
{
  SIGNAL_ABOUT_TO_FINISH,
  LAST_SIGNAL
};

static void gst_play_bin_class_init (GstPlayBinClass * klass);
static void gst_play_bin_init (GstPlayBin * play_bin);
static void gst_play_bin_finalize (GObject * object);

static void gst_play_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * spec);
static void gst_play_bin_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * spec);

static GstStateChangeReturn gst_play_bin_change_state (GstElement * element,
    GstStateChange transition);

static void gst_play_bin_handle_message (GstBin * bin, GstMessage * message);

static gboolean setup_next_source (GstPlayBin * playbin);

static GstElementClass *parent_class;

static guint gst_play_bin_signals[LAST_SIGNAL] = { 0 };

static const GstElementDetails gst_play_bin_details =
GST_ELEMENT_DETAILS ("Player Bin 2",
    "Generic/Bin/Player",
    "Autoplug and play media from an uri",
    "Wim Taymans <wim.taymans@gmail.com>");

static GType
gst_play_bin_get_type (void)
{
  static GType gst_play_bin_type = 0;

  if (!gst_play_bin_type) {
    static const GTypeInfo gst_play_bin_info = {
      sizeof (GstPlayBinClass),
      NULL,
      NULL,
      (GClassInitFunc) gst_play_bin_class_init,
      NULL,
      NULL,
      sizeof (GstPlayBin),
      0,
      (GInstanceInitFunc) gst_play_bin_init,
      NULL
    };

    gst_play_bin_type = g_type_register_static (GST_TYPE_PIPELINE,
        "GstPlayBin2", &gst_play_bin_info, 0);
  }

  return gst_play_bin_type;
}

static void
gst_play_bin_class_init (GstPlayBinClass * klass)
{
  GObjectClass *gobject_klass;
  GstElementClass *gstelement_klass;
  GstBinClass *gstbin_klass;

  gobject_klass = (GObjectClass *) klass;
  gstelement_klass = (GstElementClass *) klass;
  gstbin_klass = (GstBinClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_klass->set_property = gst_play_bin_set_property;
  gobject_klass->get_property = gst_play_bin_get_property;

  gobject_klass->finalize = GST_DEBUG_FUNCPTR (gst_play_bin_finalize);

  g_object_class_install_property (gobject_klass, PROP_URI,
      g_param_spec_string ("uri", "URI", "URI of the media to play",
          NULL, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_klass, PROP_SUBURI,
      g_param_spec_string ("suburi", ".sub-URI", "Optional URI of a subtitle",
          NULL, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_klass, PROP_NEXT_URI,
      g_param_spec_string ("next-uri", "Next URI",
          "URI of the next media to play", NULL, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_klass, PROP_NEXT_SUBURI,
      g_param_spec_string ("next-suburi", "Next .sub-URI",
          "Optional URI of a next subtitle", NULL, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_klass, PROP_STREAMINFO,
      g_param_spec_value_array ("stream-info",
          "StreamInfo GValueArray", "value array of streaminfo",
          g_param_spec_object ("streaminfo", "StreamInfo", "Streaminfo object",
              GST_TYPE_STREAM_INFO, G_PARAM_READABLE), G_PARAM_READABLE));

  g_object_class_install_property (gobject_klass, PROP_SOURCE,
      g_param_spec_object ("source", "Source", "Source element",
          GST_TYPE_ELEMENT, G_PARAM_READABLE));

  g_object_class_install_property (gobject_klass, PROP_CURRENT_VIDEO,
      g_param_spec_int ("current-video", "Current video",
          "Currently playing video stream (-1 = none)",
          -1, G_MAXINT, -1, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_klass, PROP_CURRENT_AUDIO,
      g_param_spec_int ("current-audio", "Current audio",
          "Currently playing audio stream (-1 = none)",
          -1, G_MAXINT, -1, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_klass, PROP_CURRENT_TEXT,
      g_param_spec_int ("current-text", "Current text",
          "Currently playing text stream (-1 = none)",
          -1, G_MAXINT, -1, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_klass, PROP_SUBTITLE_ENCODING,
      g_param_spec_string ("subtitle-encoding", "subtitle encoding",
          "Encoding to assume if input subtitles are not in UTF-8 encoding. "
          "If not set, the GST_SUBTITLE_ENCODING environment variable will "
          "be checked for an encoding to use. If that is not set either, "
          "ISO-8859-15 will be assumed.", NULL, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_klass, PROP_VIDEO_SINK,
      g_param_spec_object ("video-sink", "Video Sink",
          "the video output element to use (NULL = default sink)",
          GST_TYPE_ELEMENT, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_klass, PROP_AUDIO_SINK,
      g_param_spec_object ("audio-sink", "Audio Sink",
          "the audio output element to use (NULL = default sink)",
          GST_TYPE_ELEMENT, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_klass, PROP_VIS_PLUGIN,
      g_param_spec_object ("vis-plugin", "Vis plugin",
          "the visualization element to use (NULL = none)",
          GST_TYPE_ELEMENT, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_klass, PROP_VOLUME,
      g_param_spec_double ("volume", "volume", "volume",
          0.0, VOLUME_MAX_DOUBLE, 1.0, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_klass, PROP_FRAME,
      gst_param_spec_mini_object ("frame", "Frame",
          "The last frame (NULL = no video available)",
          GST_TYPE_BUFFER, G_PARAM_READABLE));
  g_object_class_install_property (gobject_klass, PROP_FONT_DESC,
      g_param_spec_string ("subtitle-font-desc",
          "Subtitle font description",
          "Pango font description of font "
          "to be used for subtitle rendering", NULL, G_PARAM_WRITABLE));

  g_object_class_install_property (gobject_klass, PROP_CONNECTION_SPEED,
      g_param_spec_uint ("connection-speed", "Connection Speed",
          "Network connection speed in kbps (0 = unknown)",
          0, G_MAXUINT, CONNECTION_SPEED_DEFAULT, G_PARAM_READWRITE));
  /**
   * GstPlayBin::about-to-finish:
   *
   * This signal is emitted when the current uri is about to finish. You can
   * set the next-uri and next-suburi to make sure that playback continues.
   */
  gst_play_bin_signals[SIGNAL_ABOUT_TO_FINISH] =
      g_signal_new ("about-to-finish", G_TYPE_FROM_CLASS (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstPlayBinClass, about_to_finish), NULL, NULL,
      gst_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

  gst_element_class_set_details (gstelement_klass, &gst_play_bin_details);

  gstelement_klass->change_state =
      GST_DEBUG_FUNCPTR (gst_play_bin_change_state);

  gstbin_klass->handle_message =
      GST_DEBUG_FUNCPTR (gst_play_bin_handle_message);
}

static void
init_group (GstPlayBin * playbin, GstSourceGroup * group)
{
  /* init selectors */
  group->playbin = playbin;
  group->selector[0].media = "audio/x-raw-";
  group->selector[0].type = GST_PLAY_SINK_TYPE_AUDIO_RAW;
  group->selector[0].mode = GST_PLAY_SINK_MODE_AUDIO;
  group->selector[1].media = "audio/";
  group->selector[1].type = GST_PLAY_SINK_TYPE_AUDIO;
  group->selector[1].mode = GST_PLAY_SINK_MODE_AUDIO;
  group->selector[2].media = "video/x-raw-";
  group->selector[2].type = GST_PLAY_SINK_TYPE_VIDEO_RAW;
  group->selector[2].mode = GST_PLAY_SINK_MODE_VIDEO;
  group->selector[3].media = "video/";
  group->selector[3].type = GST_PLAY_SINK_TYPE_VIDEO;
  group->selector[3].mode = GST_PLAY_SINK_MODE_VIDEO;
  group->selector[4].media = "text/";
  group->selector[4].type = GST_PLAY_SINK_TYPE_TEXT;
  group->selector[4].mode = GST_PLAY_SINK_MODE_TEXT;
}

static void
gst_play_bin_init (GstPlayBin * playbin)
{
  GstFactoryListType type;

  /* init groups */
  playbin->curr_group = &playbin->groups[0];
  playbin->next_group = &playbin->groups[1];
  init_group (playbin, &playbin->groups[0]);
  init_group (playbin, &playbin->groups[1]);

  /* first filter out the interesting element factories */
  type = GST_FACTORY_LIST_DECODER | GST_FACTORY_LIST_SINK;
  playbin->elements = gst_factory_list_get_elements (type);
  gst_factory_list_debug (playbin->elements);

  /* get the caps */
}

static void
gst_play_bin_finalize (GObject * object)
{
  GstPlayBin *play_bin;

  play_bin = GST_PLAY_BIN (object);

  g_value_array_free (play_bin->elements);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_play_bin_set_uri (GstPlayBin * play_bin, const gchar * uri)
{
  GstSourceGroup *group;

  if (uri == NULL) {
    g_warning ("cannot set NULL uri");
    return;
  }

  GST_OBJECT_LOCK (play_bin);
  group = play_bin->next_group;

  /* if we have no previous uri, or the new uri is different from the
   * old one, replug */
  g_free (group->uri);
  group->uri = g_strdup (uri);
  group->valid = TRUE;

  GST_DEBUG ("setting new uri to %s", uri);
  GST_OBJECT_UNLOCK (play_bin);
}

static void
gst_play_bin_set_suburi (GstPlayBin * play_bin, const gchar * suburi)
{
  GstSourceGroup *group;

  GST_OBJECT_LOCK (play_bin);
  group = play_bin->next_group;

  if ((!suburi && !group->suburi) ||
      (suburi && group->suburi && !strcmp (group->suburi, suburi)))
    goto done;

  g_free (group->suburi);
  group->suburi = g_strdup (suburi);

  GST_DEBUG ("setting new .sub uri to %s", suburi);

done:
  GST_OBJECT_UNLOCK (play_bin);
}

static void
gst_play_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstPlayBin *play_bin;

  play_bin = GST_PLAY_BIN (object);

  switch (prop_id) {
    case PROP_URI:
      gst_play_bin_set_uri (play_bin, g_value_get_string (value));
      break;
    case PROP_SUBURI:
      gst_play_bin_set_suburi (play_bin, g_value_get_string (value));
      break;
    case PROP_NEXT_URI:
      gst_play_bin_set_uri (play_bin, g_value_get_string (value));
      break;
    case PROP_NEXT_SUBURI:
      gst_play_bin_set_suburi (play_bin, g_value_get_string (value));
      break;
    case PROP_VIDEO_SINK:
      break;
    case PROP_AUDIO_SINK:
      break;
    case PROP_VIS_PLUGIN:
      break;
    case PROP_VOLUME:
      break;
    case PROP_FONT_DESC:
      break;
    case PROP_CONNECTION_SPEED:
      GST_OBJECT_LOCK (play_bin);
      play_bin->connection_speed = g_value_get_uint (value) * 1000;
      GST_OBJECT_UNLOCK (play_bin);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_play_bin_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstPlayBin *play_bin;

  play_bin = GST_PLAY_BIN (object);

  switch (prop_id) {
    case PROP_URI:
      GST_OBJECT_LOCK (play_bin);
      /* get the currently playing group first, then the queued one */
      if (play_bin->curr_group)
        g_value_set_string (value, play_bin->curr_group->uri);
      else
        g_value_set_string (value, play_bin->next_group->uri);
      GST_OBJECT_UNLOCK (play_bin);
      break;
    case PROP_SUBURI:
      GST_OBJECT_LOCK (play_bin);
      /* get the currently playing group first, then the queued one */
      if (play_bin->curr_group)
        g_value_set_string (value, play_bin->curr_group->suburi);
      else
        g_value_set_string (value, play_bin->next_group->suburi);
      GST_OBJECT_UNLOCK (play_bin);
      break;
    case PROP_NEXT_URI:
      GST_OBJECT_LOCK (play_bin);
      g_value_set_string (value, play_bin->next_group->uri);
      GST_OBJECT_UNLOCK (play_bin);
      break;
    case PROP_NEXT_SUBURI:
      GST_OBJECT_LOCK (play_bin);
      g_value_set_string (value, play_bin->next_group->suburi);
      GST_OBJECT_UNLOCK (play_bin);
      break;
    case PROP_VIDEO_SINK:
      break;
    case PROP_AUDIO_SINK:
      break;
    case PROP_VIS_PLUGIN:
      break;
    case PROP_VOLUME:
      break;
    case PROP_FRAME:
      break;
    case PROP_CONNECTION_SPEED:
      GST_OBJECT_LOCK (play_bin);
      g_value_set_uint (value, play_bin->connection_speed / 1000);
      GST_OBJECT_UNLOCK (play_bin);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* mime types we are not handling on purpose right now, don't post a
 * missing-plugin message for these */
static const gchar *blacklisted_mimes[] = {
  "video/x-dvd-subpicture", NULL
};

static void
gst_play_bin_handle_message (GstBin * bin, GstMessage * msg)
{
  if (gst_is_missing_plugin_message (msg)) {
    gchar *detail;
    guint i;

    detail = gst_missing_plugin_message_get_installer_detail (msg);
    for (i = 0; detail != NULL && blacklisted_mimes[i] != NULL; ++i) {
      if (strstr (detail, "|decoder-") && strstr (detail, blacklisted_mimes[i])) {
        GST_LOG_OBJECT (bin, "suppressing message %" GST_PTR_FORMAT, msg);
        gst_message_unref (msg);
        g_free (detail);
        return;
      }
    }
    g_free (detail);
  }
  GST_BIN_CLASS (parent_class)->handle_message (bin, msg);
}

/* this function is called when a new pad is added to decodebin. We check the
 * type of the pad and add it to the selecter element of the group. 
 */
static void
pad_added_cb (GstElement * decodebin, GstPad * pad, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GstCaps *caps;
  const GstStructure *s;
  const gchar *name;
  GstPad *sinkpad;
  GstPadLinkReturn res;
  GstSourceSelect *select = NULL;
  gint i;

  playbin = group->playbin;

  caps = gst_pad_get_caps (pad);
  s = gst_caps_get_structure (caps, 0);
  name = gst_structure_get_name (s);

  GST_DEBUG_OBJECT (playbin,
      "pad %s:%s with caps %" GST_PTR_FORMAT " added in group %p",
      GST_DEBUG_PAD_NAME (pad), caps, group);

  /* major type of the pad, this determines the selector to use */
  for (i = 0; i < 3; i++) {
    if (g_str_has_prefix (name, group->selector[i].media)) {
      select = &group->selector[i];
      break;
    }
  }
  /* no selector found for the media type, don't bother linking it to a
   * selector. This will leave the pad unlinked and thus ignored. */
  if (select == NULL)
    goto unknown_type;

  if (select->selector == NULL) {
    /* no selector, create one */
    GST_DEBUG_OBJECT (playbin, "creating new selector");
    select->selector = g_object_new (GST_TYPE_STREAM_SELECTOR, NULL);
    if (select->selector == NULL)
      goto no_selector;

    GST_DEBUG_OBJECT (playbin, "adding new selector %p", select->selector);
    gst_bin_add (GST_BIN_CAST (playbin), select->selector);
    gst_element_set_state (select->selector, GST_STATE_PAUSED);

    /* save source pad */
    select->srcpad = gst_element_get_pad (select->selector, "src");
  }

  /* get sinkpad for the new stream */
  if ((sinkpad = gst_element_get_request_pad (select->selector, "sink%d"))) {
    GST_DEBUG_OBJECT (playbin, "got pad %s:%s from selector",
        GST_DEBUG_PAD_NAME (sinkpad));
    res = gst_pad_link (pad, sinkpad);
    if (GST_PAD_LINK_FAILED (res))
      goto link_failed;

    /* store selector pad so we can release it */
    g_object_set_data (G_OBJECT (pad), "playbin2.selector", sinkpad);
  }
  GST_DEBUG_OBJECT (playbin, "linked pad %s:%s to selector %p",
      GST_DEBUG_PAD_NAME (pad), select->selector);

  return;

  /* ERRORS */
unknown_type:
  {
    GST_ERROR_OBJECT (playbin, "unknown type %s for pad %s:%s",
        name, GST_DEBUG_PAD_NAME (pad));
    return;
  }
no_selector:
  {
    GST_ERROR_OBJECT (playbin, "could not create selector for pad %s:%s",
        GST_DEBUG_PAD_NAME (pad));
    return;
  }
link_failed:
  {
    GST_ERROR_OBJECT (playbin,
        "failed to link pad %s:%s to selector, reason %d",
        GST_DEBUG_PAD_NAME (pad), res);
    return;
  }
}

/* called when a pad is removed form the uridecodebin. We unlink the pad from
 * the selector. This will make the selector select a new pad. */
static void
pad_removed_cb (GstElement * decodebin, GstPad * pad, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GstPad *peer;
  GstElement *selector;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin,
      "pad %s:%s removed from group %p", GST_DEBUG_PAD_NAME (pad), group);

  /* get the selector sinkpad */
  peer = g_object_get_data (G_OBJECT (pad), "playbin2.selector");
  if (!peer)
    goto not_linked;

  /* unlink the pad now (can fail, the pad is unlinked before it's removed) */
  gst_pad_unlink (pad, peer);

  /* get selector */
  selector = GST_ELEMENT_CAST (gst_pad_get_parent (peer));

  /* release the pad to the selector, this will make the selector choose a new
   * pad. */
  gst_element_release_request_pad (selector, peer);

  gst_object_unref (selector);

  return;

  /* ERRORS */
not_linked:
  {
    GST_DEBUG_OBJECT (playbin, "pad not linked");
    return;
  }
}

/* we get called when all pads are available and we must connect the sinks to
 * them.
 * The main purpose of the code is to see if we have video/audio and subtitles
 * and pick the right pipelines to display them.
 *
 * The selectors installed on the group tell us about the presence of
 * audio/video and subtitle streams. This allows us to see if we need
 * visualisation, video or/and audio.
 */
static void
no_more_pads_cb (GstElement * decodebin, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GstPlaySinkMode mode = 0;
  GstPadLinkReturn res;
  gint i;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin, "no more pads in group %p", group);

  for (i = 0; i < 3; i++) {
    GstSourceSelect *select = &group->selector[i];

    if (select->selector) {
      select->sinkpad =
          gst_play_sink_request_pad (playbin->playsink, select->type);
      res = gst_pad_link (select->srcpad, select->sinkpad);
      GST_DEBUG_OBJECT (playbin, "linking type %s: %d", select->media, res);
      mode |= select->mode;
    }
  }
  /* configure the modes now */
  gst_play_sink_set_mode (playbin->playsink, mode);
}

/* send an EOS event to all of the selectors */
static void
perform_eos (GstPlayBin * playbin, GstSourceGroup * group)
{
  GstEvent *event;
  gint i;

  GST_DEBUG_OBJECT (playbin, "doing EOS in group %p", group);

  event = gst_event_new_eos ();
  for (i = 0; i < 3; i++) {
    GstSourceSelect *select = &group->selector[i];

    if (select->selector) {
      GST_DEBUG_OBJECT (playbin, "send EOS in selector %s", select->media);
      gst_event_ref (event);
      gst_pad_push_event (select->srcpad, event);
    }
  }
  gst_event_unref (event);
}

static void
drained_cb (GstElement * decodebin, GstSourceGroup * group)
{
  GstPlayBin *playbin;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin, "about to finish in group %p", group);

  /* after this call, we should have a next group to activate or we EOS */
  g_signal_emit (G_OBJECT (playbin),
      gst_play_bin_signals[SIGNAL_ABOUT_TO_FINISH], 0, NULL);

  /* now activate the next group. If the app did not set a next-uri, this will
   * fail and we can do EOS */
  if (!setup_next_source (playbin)) {
    perform_eos (playbin, group);
  }
}

/* Called when we must provide a list of factories to plug to @pad with @caps.
 * We first check if we have a sink that can handle the format and if we do, we
 * return NULL, to expose the pad. If we have no sink (or the sink does not
 * work), we return the list of elements that can connect. */
static GValueArray *
autoplug_factories_cb (GstElement * decodebin, GstPad * pad,
    GstCaps * caps, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GValueArray *result;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin, "factories group %p for %s:%s, %" GST_PTR_FORMAT,
      group, GST_DEBUG_PAD_NAME (pad), caps);

  /* filter out the elements based on the caps. */
  result = gst_factory_list_filter (playbin->elements, caps);

  GST_DEBUG_OBJECT (playbin, "found factories %p", result);
  gst_factory_list_debug (result);

  return result;
}

/* We are asked to select an element. See if the next element to check
 * is a sink. If this is the case, we see if the sink works by setting it to
 * READY. If the sink works, we return -2 to make decodebin expose the raw pad
 * so that we can setup the mixers. */
static GstAutoplugSelectResult
autoplug_select_cb (GstElement * decodebin, GstPad * pad,
    GstCaps * caps, GstElementFactory * factory, GstSourceGroup * group)
{
  GstPlayBin *playbin;
  GstElement *element;
  const gchar *klass;

  playbin = group->playbin;

  GST_DEBUG_OBJECT (playbin, "select group %p for %s:%s, %" GST_PTR_FORMAT,
      group, GST_DEBUG_PAD_NAME (pad), caps);

  GST_DEBUG_OBJECT (playbin, "checking factory %s",
      GST_PLUGIN_FEATURE_NAME (factory));

  /* if it's not a sink, we just make decodebin try it */
  if (!gst_factory_list_is_type (factory, GST_FACTORY_LIST_SINK))
    return GST_AUTOPLUG_SELECT_TRY;

  /* it's a sink, see if an instance of it actually works */
  GST_DEBUG_OBJECT (playbin, "we found a sink");

  if ((element = gst_element_factory_create (factory, NULL)) == NULL) {
    GST_WARNING_OBJECT (playbin, "Could not create an element from %s",
        gst_plugin_feature_get_name (GST_PLUGIN_FEATURE (factory)));
    return GST_AUTOPLUG_SELECT_SKIP;
  }

  /* ... activate it ... We do this before adding it to the bin so that we
   * don't accidentally make it post error messages that will stop
   * everything. */
  if ((gst_element_set_state (element,
              GST_STATE_READY)) == GST_STATE_CHANGE_FAILURE) {
    GST_WARNING_OBJECT (playbin, "Couldn't set %s to READY",
        GST_ELEMENT_NAME (element));
    gst_object_unref (element);
    return GST_AUTOPLUG_SELECT_SKIP;
  }

  /* at this point, we have the sink working, configure it in playsink */
  klass = gst_element_factory_get_klass (factory);

  /* get klass to figure out if it's audio or video */
  if (strstr (klass, "Audio")) {
    GST_DEBUG_OBJECT (playbin, "configure audio sink");
    gst_play_sink_set_audio_sink (playbin->playsink, element);
  } else if (strstr (klass, "Video")) {
    GST_DEBUG_OBJECT (playbin, "configure video sink");
    gst_play_sink_set_video_sink (playbin->playsink, element);
  } else {
    GST_WARNING_OBJECT (playbin, "unknown sink klass %s found", klass);
  }

  /* tell decodebin to expose the pad because we are going to use this
   * sink */
  GST_DEBUG_OBJECT (playbin, "we found a working sink, expose pad");

  return GST_AUTOPLUG_SELECT_EXPOSE;
}

/* unlink a group of uridecodebins from the sink */
static void
unlink_group (GstPlayBin * playbin, GstSourceGroup * group)
{
  gint i;

  GST_DEBUG_OBJECT (playbin, "unlinking group %p", group);

  for (i = 0; i < 3; i++) {
    GstSourceSelect *select = &group->selector[i];

    if (!select->selector)
      continue;

    GST_DEBUG_OBJECT (playbin, "unlinking selector %s", select->media);
    gst_pad_unlink (select->srcpad, select->sinkpad);

    /* release back */
    gst_play_sink_release_pad (playbin->playsink, select->sinkpad);
    select->sinkpad = NULL;

    gst_object_unref (select->srcpad);
    select->srcpad = NULL;

    gst_element_set_state (select->selector, GST_STATE_NULL);
    gst_bin_remove (GST_BIN_CAST (playbin), select->selector);
    select->selector = NULL;
  }
  group->valid = FALSE;
}

static gboolean
activate_group (GstPlayBin * playbin, GstSourceGroup * group)
{
  GstElement *uridecodebin;

  uridecodebin = gst_element_factory_make ("uridecodebin", NULL);
  if (!uridecodebin)
    goto no_decodebin;

  /* configure uri */
  g_object_set (uridecodebin, "uri", group->uri, NULL);

  /* connect pads and other things */
  g_signal_connect (uridecodebin, "pad-added", G_CALLBACK (pad_added_cb),
      group);
  g_signal_connect (uridecodebin, "pad-removed", G_CALLBACK (pad_removed_cb),
      group);
  g_signal_connect (uridecodebin, "no-more-pads", G_CALLBACK (no_more_pads_cb),
      group);
  /* is called when the uridecodebin is out of data and we can switch to the
   * next uri */
  g_signal_connect (uridecodebin, "drained", G_CALLBACK (drained_cb), group);

  /* will be called when a new media type is found. We return a list of decoders
   * including sinks for decodebin to try */
  g_signal_connect (uridecodebin, "autoplug-factories",
      G_CALLBACK (autoplug_factories_cb), group);

  g_signal_connect (uridecodebin, "autoplug-select",
      G_CALLBACK (autoplug_select_cb), group);

  /*  */
  gst_bin_add (GST_BIN_CAST (playbin), uridecodebin);
  group->uridecodebin = uridecodebin;

  gst_element_set_state (uridecodebin, GST_STATE_PAUSED);

  return TRUE;

  /* ERRORS */
no_decodebin:
  {
    return FALSE;
  }
}

/* setup the next group to play */
static gboolean
setup_next_source (GstPlayBin * playbin)
{
  GstSourceGroup *new_group, *old_group;

  GST_DEBUG_OBJECT (playbin, "setup sources");

  /* see if there is a next group */
  new_group = playbin->next_group;
  if (!new_group || !new_group->valid)
    goto no_next_group;

  /* first unlink the current source, if any */
  old_group = playbin->curr_group;
  if (old_group && old_group->valid) {
    /* unlink our pads with the sink */
    unlink_group (playbin, old_group);
  }

  /* activate the new group */
  if (!activate_group (playbin, new_group))
    goto activate_failed;

  /* swap old and new */
  playbin->curr_group = new_group;
  playbin->next_group = old_group;

  return TRUE;

  /* ERRORS */
no_next_group:
  {
    GST_DEBUG_OBJECT (playbin, "no next group");
    return FALSE;
  }
activate_failed:
  {
    GST_DEBUG_OBJECT (playbin, "activate failed");
    return FALSE;
  }
}

static GstStateChangeReturn
gst_play_bin_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstPlayBin *play_bin;

  play_bin = GST_PLAY_BIN (element);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      if (play_bin->playsink == NULL) {
        play_bin->playsink = g_object_new (GST_TYPE_PLAY_SINK, NULL);
        gst_bin_add (GST_BIN_CAST (play_bin),
            GST_ELEMENT_CAST (play_bin->playsink));
      }
      if (!setup_next_source (play_bin))
        goto source_failed;
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      break;
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      /* FIXME Release audio device when we implement that */
      break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      if (play_bin->playsink) {
        gst_bin_remove (GST_BIN_CAST (play_bin),
            GST_ELEMENT_CAST (play_bin->playsink));
        play_bin->playsink = NULL;
      }
      break;
    default:
      break;
  }

  return ret;

  /* ERRORS */
source_failed:
  {
    return GST_STATE_CHANGE_FAILURE;
  }
}

gboolean
gst_play_bin2_plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_play_bin_debug, "playbin2", 0, "play bin");

  return gst_element_register (plugin, "playbin2", GST_RANK_NONE,
      GST_TYPE_PLAY_BIN);
}
