using AutoUpdaterDotNET;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Text.Json;
using System.Threading;
using System.Globalization;

namespace elaphureLink.Wpf.Core
{
    class WpfUpdate
    {
        private static readonly NLog.Logger Logger = NLog.LogManager.GetCurrentClassLogger();

        public static async Task UpdateProgram()
        {
            Logger.Info("Start the update process");

            AutoUpdater.ParseUpdateInfoEvent += AutoUpdaterOnParseUpdateInfoEvent;
            AutoUpdater.ReportErrors = true;
            AutoUpdater.HttpUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36";
            await Task.Run(() => AutoUpdater.Start("https://api.github.com/repos/windowsair/elaphureLink/releases/latest"));
        }

        private static void AutoUpdaterOnParseUpdateInfoEvent(ParseUpdateInfoEventArgs args)
        {
            var jsonObject = JsonDocument.Parse(args.RemoteData);

            string versionName = jsonObject.RootElement.GetProperty("tag_name").GetString();
            if (versionName.StartsWith("v") || versionName.StartsWith("V"))
            {
                versionName = versionName.Substring(1); // "v0.0.0.1" => "0.0.0.1"
            }

            string downloadAddress = "";

            var assetsArray = jsonObject.RootElement.GetProperty("assets");
            foreach (var item in assetsArray.EnumerateArray())
            {
                string fileName = item.GetProperty("name").GetString();
                if (!fileName.Contains("release"))
                {
                    continue;
                }

                if (System.Environment.Is64BitProcess && !fileName.Contains("x64"))
                {
                    continue;
                }

                downloadAddress = item.GetProperty("browser_download_url").GetString();
                break;
            }

            args.UpdateInfo = new UpdateInfoEventArgs
            {
                CurrentVersion = versionName,
                ChangelogURL = "https://github.com/windowsair/elaphureLink/releases",
                DownloadURL = downloadAddress,
                Mandatory = new Mandatory
                {
                    Value = false,
                    MinimumVersion = "0.0.0.1"
                }
            };

        }
    }
}
