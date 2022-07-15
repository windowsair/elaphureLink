using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Win32;

namespace elaphureLink.Wpf.Core.Driver
{
    class DriverService
    {
        private static readonly NLog.Logger Logger = NLog.LogManager.GetCurrentClassLogger();

        public static async Task InstallDriver(string driverPath)
        {
            ProcessStartInfo info = new ProcessStartInfo();
            info.FileName = "DriverInstallHelper.exe";
            info.UseShellExecute = true;
            info.Verb = "runas"; // Provides Run as Administrator

            string installArg = String.Format("\"{0}\"", driverPath);
            info.Arguments = installArg;

            Logger.Info("Start to install driver");

            Process newProcess = null;
            try
            {
                newProcess = Process.Start(info);
            }
            catch
            {
                // UAC denied
                Logger.Warn("User aborts the driver installation process");
            }

            if (newProcess != null)
            {
                // for .Net 4.x
                while (!newProcess.HasExited)
                    await Task.Delay(100);

                int installReturnCode = newProcess.ExitCode;
                if (installReturnCode == 0)
                {
                    Logger.Info("The driver was successfully installed");
                }
                else
                {
                    Logger.Error("Failed to install driver");
                }
            }
        }

        public static string getKeilInstallPath()
        {
            return getSoftWareInstallPath("Keil μVision4");
        }

        private static string getSoftWareInstallPath(string findByName)
        {
            string displayName;
            string registryKey = @"SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall";

            // FIXME: location not exist


            // 64 bits computer
            RegistryKey key64 = RegistryKey.OpenBaseKey(
                RegistryHive.LocalMachine,
                RegistryView.Registry64
            );
            RegistryKey key = key64.OpenSubKey(registryKey);

            if (key != null)
            {
                foreach (
                    RegistryKey subkey in key.GetSubKeyNames()
                        .Select(keyName => key.OpenSubKey(keyName))
                )
                {
                    displayName = subkey.GetValue("DisplayName") as string;
                    if (displayName != null && displayName.Contains(findByName))
                    {

                        var InstallPath = subkey.GetValue("LastInstallDir");
                        if (InstallPath != null)
                        {
                            return InstallPath.ToString();
                        }
                    }
                }
                key.Close();
            }

            return null;
        }
    }
}
