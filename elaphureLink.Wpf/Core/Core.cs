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

        private static SettingsService _SettingService = new SettingsService();

        ////
        [DllImport(
            "elaphureLinkProxy.dll",
            EntryPoint = "el_proxy_init",
            CallingConvention = CallingConvention.Cdecl
        )]
        private static extern System.Int32 el_proxy_init();

        [DllImport(
            "elaphureLinkProxy.dll",
            EntryPoint = "el_proxy_start_with_address",
            CallingConvention = CallingConvention.Cdecl,
            CharSet = CharSet.Ansi
        )]
        private static extern System.Int32 el_proxy_start_with_address(string address);

        [DllImport(
            "elaphureLinkProxy.dll",
            EntryPoint = "el_proxy_stop",
            CallingConvention = CallingConvention.Cdecl
        )]
        private static extern void el_proxy_stop();

        // on disconnect callback type
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        private delegate void OnProxyDisconnectCallbackType(
            [MarshalAs(UnmanagedType.LPStr)] string messageFromProxy
        );

        // set on disconnect callback function
        [DllImport(
            "elaphureLinkProxy.dll",
            EntryPoint = "el_proxy_set_on_disconnect_callback",
            CallingConvention = CallingConvention.Cdecl
        )]
        private static extern void el_proxy_set_on_disconnect_callback(
            [MarshalAs(UnmanagedType.FunctionPtr)] OnProxyDisconnectCallbackType callbackPointer
        );

        private static OnProxyDisconnectCallbackType OnProxyDisconnectCallback = (msg) =>
        {
            Logger.Info(msg);

            WeakReferenceMessenger.Default.Send(new ProxyStatusChangedMessage(false));
        };

        [StructLayout(LayoutKind.Explicit)]
        public struct proxyConfig
        {
            [FieldOffset(0)]
            public byte enable_vendor_command;
        }

        [DllImport(
            "elaphureLinkProxy.dll",
            EntryPoint = "el_proxy_change_config",
            CallingConvention = CallingConvention.Cdecl
        )]
        private static extern void el_proxy_change_config(ref proxyConfig config);

        //////

        public static async Task ChangeProxyConfigAsync(bool enable_vendor_command)
        {
            await Task.Factory.StartNew(() =>
            {
                proxyConfig config = new proxyConfig
                {
                    enable_vendor_command = Convert.ToByte(enable_vendor_command)
                };

                el_proxy_change_config(ref config);
            });
        }

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

                proxyConfig config = new proxyConfig
                {
                    enable_vendor_command = Convert.ToByte(
                        _SettingService.GetValue<bool>("EnableVendorCommand"))
                };
                el_proxy_change_config(ref config);

                ret = el_proxy_start_with_address(deviceAddress);
                if (ret != 0)
                {
                    Logger.Error("Failed to start proxy, perhaps an invalid address?");

                    WeakReferenceMessenger.Default.Send(new ProxyStatusChangedMessage(false));
                    return;
                }

                el_proxy_set_on_disconnect_callback(OnProxyDisconnectCallback);

                Logger.Info("Proxy has started successfully");

                WeakReferenceMessenger.Default.Send(new ProxyStatusChangedMessage(true));
            });
        }

        public static async Task StopProxyAsync()
        {
            WeakReferenceMessenger.Default.Send(new ProxyStatusChangedMessage(false));
            await Task.Factory.StartNew(() => el_proxy_stop());
        }
    }
}
