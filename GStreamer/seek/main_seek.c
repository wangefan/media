#include <string.h>
#include <stdio.h>
#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

typedef struct _CustomData
{
  GstElement *pipeline;
  GstElement *video_sink;
  GMainLoop *loop;

  gboolean playing;             /* Playing or Paused */
  gdouble rate;                 /* Current playback rate (can be negative) */
} CustomData;

/* Send seek event to change rate */
static void
send_seek_event (CustomData * data)
{
  gint64 position;
  GstEvent *seek_event;

  /* Obtain the current position, needed for the seek event */
  if (!gst_element_query_position (data->pipeline, GST_FORMAT_TIME, &position)) {
    g_printerr ("Unable to retrieve current position.\n");
    return;
  }

  /* Create the seek event */
  if (data->rate > 0) {
    seek_event =
        gst_event_new_seek (data->rate, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
        position, GST_SEEK_TYPE_END, 0);
  } else {
    seek_event =
        gst_event_new_seek (data->rate, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET, 0,
        GST_SEEK_TYPE_SET, position);
  }

  if (data->video_sink == NULL) {
    /* If we have not done so, obtain the sink through which we will send the seek events */
    g_object_get (data->pipeline, "video-sink", &data->video_sink, NULL);
    //g_object_set(data->video_sink, "frame-step-on-preroll", TRUE, NULL);
  }

  /* Send the event */
  gst_element_send_event (data->video_sink, seek_event);

  //g_print ("Before force to update state, data->playing= %d\n", data->playing);
  /*if (!data->playing) {
    gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
    g_usleep(10);
    gst_element_set_state(data->pipeline, GST_STATE_PAUSED);
    g_print ("Force to update state\n");
  }*/

  g_print ("Current rate: %g\n", data->rate);
}

/* Process keyboard input */
static gboolean
handle_keyboard (GIOChannel * source, GIOCondition cond, CustomData * data)
{
  g_print ("handle_keyboard begin");
  gchar *str = NULL;

  if (g_io_channel_read_line (source, &str, NULL, NULL,
          NULL) != G_IO_STATUS_NORMAL) {
    return TRUE;
  }

  if (data->video_sink != NULL) {
    gboolean frame_step_on_preroll;
    g_object_get(data->video_sink, "frame-step-on-preroll", &frame_step_on_preroll, NULL);
    g_print("frame-step-on-preroll: %s\n", frame_step_on_preroll ? "TRUE" : "FALSE");
  }

  switch (g_ascii_tolower (str[0])) {
    case 'p':
      data->playing = !data->playing;
      gst_element_set_state (data->pipeline,
          data->playing ? GST_STATE_PLAYING : GST_STATE_PAUSED);
      g_print ("Setting state to %s\n", data->playing ? "PLAYING" : "PAUSE");
      break;
    case 's':
      if (g_ascii_isupper (str[0])) {
        data->rate *= 2.0;
      } else {
        data->rate /= 2.0;
      }
      send_seek_event (data);
      break;
    case 'd':
      data->rate *= -1.0;
      send_seek_event (data);
      break;
    case 'n':
      if (data->video_sink == NULL) {
        /* If we have not done so, obtain the sink through which we will send the step events */
        g_object_get (data->pipeline, "video-sink", &data->video_sink, NULL);
        g_object_set(data->video_sink, "frame-step-on-preroll", TRUE, NULL);
      }

      gst_element_send_event (data->video_sink,
          gst_event_new_step (GST_FORMAT_BUFFERS, 20, ABS (data->rate), TRUE,
              FALSE));
      //g_print("Before force to update state, data->playing= %d\n",
      //        data->playing);
      /*if (!data->playing) {
        gst_element_set_state(data->pipeline, GST_STATE_PLAYING);
        g_usleep(10);
        gst_element_set_state(data->pipeline, GST_STATE_PAUSED);
        g_print("Force to update state\n");
      }*/
      g_print ("Stepping one frame\n");
      break;
    case 'q':
      g_main_loop_quit (data->loop);
      break;
    default:
      break;
  }

  g_free (str);

  return TRUE;
}

int
tutorial_main (int argc, char *argv[])
{
  if (argc != 2) {
    g_printerr("Usage: %s <file path>\n", argv[0]);
    return -1;
  }
  
  CustomData data;
  GstStateChangeReturn ret;
  GIOChannel *io_stdin;

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Initialize our data structure */
  memset (&data, 0, sizeof (data));

  /* Print usage map */
  g_print ("USAGE: Choose one of the following options, then press enter:\n"
      " 'P' to toggle between PAUSE and PLAY\n"
      " 'S' to increase playback speed, 's' to decrease playback speed\n"
      " 'D' to toggle playback direction\n"
      " 'N' to move to next frame (in the current direction, better in PAUSE)\n"
      " 'Q' to quit\n");

  /* Build the pipeline */
  /*data.pipeline =
      gst_parse_launch
      ("playbin uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm",
      NULL);*/
  gchar* absolute_path = g_canonicalize_filename(argv[1], NULL);
  if (!g_file_test(absolute_path, G_FILE_TEST_EXISTS)) {
    g_printerr("File does not exist: %s\n", absolute_path);
    g_free(absolute_path);
    return -1;
  }
  gchar *uri;
  g_print("Absolute path: %s\n", absolute_path); // Print the absolute path
  uri = g_strdup_printf("file://%s", absolute_path);
  if (!gst_uri_is_valid(uri)) {
    g_printerr("Invalid URI: %s\n", uri);
    g_free(uri);
    return -1;
  }
  g_print("uri path: %s\n", uri);

  data.pipeline =
      gst_parse_launch
      (g_strdup_printf("playbin uri=%s", uri),
      NULL);
  g_free(absolute_path);
  g_free(uri);

  /* Get the video sink and set it to westerossink */
  GstElement *video_sink = gst_element_factory_make("glimagesink", "my_sink");
  //GstElement *video_sink = gst_element_factory_make("westerossink", "my_sink");
  if (video_sink == NULL) {
    g_printerr("Failed to create westerossink.\n");
    gst_object_unref(data.pipeline);
    return -1;
  }
  g_object_set(data.pipeline, "video-sink", video_sink, NULL);
  gst_object_unref(video_sink);

  /* Add a keyboard watch so we get notified of keystrokes */
#ifdef G_OS_WIN32
  io_stdin = g_io_channel_win32_new_fd (fileno (stdin));
#else
  io_stdin = g_io_channel_unix_new (fileno (stdin));
#endif
  g_io_add_watch (io_stdin, G_IO_IN, (GIOFunc) handle_keyboard, &data);

  /* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }
  data.playing = TRUE;
  data.rate = 1.0;

  /* Create a GLib Main Loop and set it to run */
  data.loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (data.loop);

  /* Free resources */
  g_main_loop_unref (data.loop);
  g_io_channel_unref (io_stdin);
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  if (data.video_sink != NULL)
    gst_object_unref (data.video_sink);
  gst_object_unref (data.pipeline);
  return 0;
}

int
main (int argc, char *argv[])
{
#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
  return gst_macos_main ((GstMainFunc) tutorial_main, argc, argv, NULL);
#else
  return tutorial_main (argc, argv);
#endif
}
