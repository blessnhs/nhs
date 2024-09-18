﻿using Android.App;
using Android.Content;
using Android.Media;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CCA.Droid
{
    public static class VideoConverter
    {

        /**
        * This method must be called from UI thread.
        ***/
        static public async Task<Java.IO.File> ConvertFileAsync(Context context,
            Java.IO.File inputFile,
            Action<string> logger = null,
            Action<int, int> onProgress = null)
        {
            Java.IO.File ouputFile = new Java.IO.File(inputFile.CanonicalPath + ".mpg");

            ouputFile.DeleteOnExit();

            List<string> cmd = new List<string>();
            cmd.Add("-y");
            cmd.Add("-i");
            cmd.Add(inputFile.CanonicalPath);

            MediaMetadataRetriever m = new MediaMetadataRetriever();
            m.SetDataSource(inputFile.CanonicalPath);



            string rotate = m.ExtractMetadata(Android.Media.MetadataKey.VideoRotation);

            int r = 0;

            if (!string.IsNullOrWhiteSpace(rotate))
            {
                r = int.Parse(rotate);
            }

            cmd.Add("-b:v");
            cmd.Add("1M");

            cmd.Add("-b:a");
            cmd.Add("128k");


            switch (r)
            {
                case 270:
                    cmd.Add("-vf scale=-1:480,transpose=cclock");
                    break;
                case 180:
                    cmd.Add("-vf scale=-1:480,transpose=cclock,transpose=cclock");
                    break;
                case 90:
                    cmd.Add("-vf scale=480:-1,transpose=clock");
                    break;
                case 0:
                    cmd.Add("-vf scale=-1:480");
                    break;
                default:

                    break;
            }

            cmd.Add("-f");
            cmd.Add("mpeg");

            cmd.Add(ouputFile.CanonicalPath);

            string cmdParams = string.Join(" ", cmd);

            int total = 0;
            int current = 0;

            await FFMpeg.Xamarin.FFMpegLibrary.Run(
                context,
                cmdParams
                , (s) => {
                    logger?.Invoke(s);
                    int n = Extract(s, "Duration:", ",");
                    if (n != -1)
                    {
                        total = n;
                    }
                    n = Extract(s, "time=", " bitrate=");
                    if (n != -1)
                    {
                        current = n;
                        onProgress?.Invoke(current, total);
                    }
                });

            return ouputFile;
        }

        static int Extract(String text, String start, String end)
        {
            int i = text.IndexOf(start);
            if (i != -1)
            {
                text = text.Substring(i + start.Length);
                i = text.IndexOf(end);
                if (i != -1)
                {
                    text = text.Substring(0, i);
                    return parseTime(text);
                }
            }
            return -1;
        }

        static public int parseTime(String time)
        {
            time = time.Trim();
            String[] tokens = time.Split(':');
            int hours = int.Parse(tokens[0]);
            int minutes = int.Parse(tokens[1]);
            float seconds = float.Parse(tokens[2]);
            int s = (int)seconds * 100;
            return hours * 360000 + minutes * 60100 + s;
        }
    }
}