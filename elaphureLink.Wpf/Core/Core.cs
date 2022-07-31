using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Toolkit.Mvvm;
using Microsoft.Toolkit.Mvvm.ComponentModel;
using Microsoft.Toolkit.Mvvm.Input;
using Microsoft.Toolkit.Mvvm.Messaging;

using elaphureLink.Wpf.Core.Services;
using elaphureLink.Wpf.Messenger;

namespace elaphureLink.Wpf.Core
{
    class elaphureLinkCore
    {

        private static readonly NLog.Logger Logger = NLog.LogManager.GetCurrentClassLogger();

        //

        private static SettingsService _SettingService = new SettingsService();

        //
        [DllImport("elaphureLinkProxy.dll", EntryPoint = "el_proxy_init", CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 el_proxy_init();

        [DllImport("elaphureLinkProxy.dll", EntryPoint = "el_proxy_start_with_address", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        private static extern System.Int32 el_proxy_start_with_address(string address);

        [DllImport("elaphureLinkProxy.dll", EntryPoint = "el_proxy_stop", CallingConvention = CallingConvention.Cdecl)]
        private static extern void el_proxy_stop();



        //
        [DllImport("dlltest.dll", EntryPoint = "test", CallingConvention = CallingConvention.Cdecl)]
        private static extern System.Int32 test(System.Int32 value);

        [DllImport(
            "dlltest.dll",
            EntryPoint = "startCallback",
            CallingConvention = CallingConvention.Cdecl
        )]
        private static extern void startCallback(System.Int32 value);

        [DllImport(
            "dlltest.dll",
            EntryPoint = "startMainThread",
            CallingConvention = CallingConvention.Cdecl
        )]
        private static extern void startMainThread();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void ProgressCallbackMy(int value);

        private static ProgressCallbackMy my_callback = (value) =>
        {
            Console.WriteLine("Progress = {0}", value);
            // WeakReferenceMessenger.Default.Send(new ProxyStatusChangedMessage(false));
            Logger.Info("callback {0}", value);

            bool status = _SettingService.GetValue<bool>("isProxyRunning");
            _SettingService.SetValue("isProxyRunning", !status);


            return;
        };

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void strCallback([MarshalAs(UnmanagedType.LPStr)] string strFromCpp);

        private static strCallback my_str_callback = (str1) =>
        {
            bool status = _SettingService.GetValue<bool>("isProxyRunning");
            _SettingService.SetValue("isProxyRunning", !status);
            Console.WriteLine("str from cpp = {0}", str1);
            return;
        };

        //
        //
        //

        [DllImport(
            "dlltest.dll",
            EntryPoint = "setCSharpCallBack",
            CallingConvention = CallingConvention.Cdecl
        )]
        private static extern void setCSharpCallBack(
            [MarshalAs(UnmanagedType.FunctionPtr)] ProgressCallbackMy callbackPointer
        );

        [DllImport(
            "dlltest.dll",
            EntryPoint = "setCsharpStrDelegateCallback",
            CallingConvention = CallingConvention.Cdecl
        )]
        private static extern void setCsharpStrDelegateCallback(
            [MarshalAs(UnmanagedType.FunctionPtr)] strCallback callbackPointer
        );

        // printStringFromCsharp
        [DllImport(
            "dlltest.dll",
            EntryPoint = "printStringFromCsharp",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Unicode
        )]
        private static extern void printStringFromCsharp(string myString);

        public static async Task StartProxyAsync(string deviceAddress)
        {
            Logger.Info("Launch proxy in progress");

            await Task.Factory.StartNew(() =>
            {
                int ret = el_proxy_init();
                if (ret != 0)
                {
                    Logger.Error("Can not init proxy");

                    WeakReferenceMessenger.Default.Send(new ProxyStatusChangedMessage(false));
                    return;
                }
                else
                {
                    Logger.Debug("Proxy init successed");
                }


                ret = el_proxy_start_with_address(deviceAddress);
                if (ret != 0)
                {
                    Logger.Error("Failed to start proxy, perhaps an invalid address?");

                    WeakReferenceMessenger.Default.Send(new ProxyStatusChangedMessage(false));
                }
                else
                {
                    Logger.Info("Proxy has started successfully");

                    WeakReferenceMessenger.Default.Send(new ProxyStatusChangedMessage(true));
                }
            });


        }

        public static async Task StopProxyAsync()
        {
            WeakReferenceMessenger.Default.Send(new ProxyStatusChangedMessage(false));
            await Task.Factory.StartNew(() => el_proxy_stop());

        }


        public static async Task backup(string deviceAddress)
        {
            setCsharpStrDelegateCallback(my_str_callback);
            setCSharpCallBack(my_callback);

            printStringFromCsharp("123456");

            startMainThread();

            ProcessStartInfo info = new ProcessStartInfo();
            info.FileName = "cmd.exe";
            info.UseShellExecute = true;
            info.Verb = "runas"; // Provides Run as Administrator

            string cmdArg = "/c \"echo \"";
            cmdArg =
                cmdArg
                + System.IO.Path.GetDirectoryName(
                    System.Reflection.Assembly.GetEntryAssembly().Location
                );

            cmdArg = cmdArg + "\" && pause \"";
            info.Arguments = cmdArg;

            var newProcess = Process.Start(info);
            if (newProcess != null)
            {
                // for .Net 4.x
                while (!newProcess.HasExited)
                    await Task.Delay(100);
            }
        }
    }
}
