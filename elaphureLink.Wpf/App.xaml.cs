using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

using System.Threading;
using Windows.ApplicationModel.Activation;

using Microsoft.Toolkit.Mvvm.DependencyInjection;
using Microsoft.Extensions.DependencyInjection;

using elaphureLink.Wpf.Core.Services;
using elaphureLink.Wpf.ViewModel;
using Windows.Storage;
using Windows.Foundation.Collections;

using Ninject;
using Ninject.Modules;

namespace elaphureLink.Wpf
{
    class IocConfiguration : NinjectModule
    {
        public override void Load()
        {
            Bind<ISettingsService>().To<SettingsService>().InSingletonScope(); // Reuse same storage every time

            Bind<HomePageViewModel>().ToSelf().InTransientScope(); // Create new instance every time
            Bind<InfoPageViewModel>().ToSelf().InTransientScope();

        }
    }

    public partial class App : Application
    {
        public new static App Current => (App)Application.Current;

        public IServiceProvider Services { get; }

        private static IServiceProvider ConfigureServices()
        {
            var services = new ServiceCollection();

            services.AddSingleton<ISettingsService, SettingsService>();

            services.AddTransient<HomePageViewModel>();
            services.AddTransient<InfoPageViewModel>();

            return services.BuildServiceProvider();
        }

        public static class IocKernel
        {
            private static StandardKernel _kernel;

            public static T Get<T>()
            {
                return _kernel.Get<T>();
            }

            public static void Initialize(params INinjectModule[] modules)
            {
                if (_kernel == null)
                {
                    _kernel = new StandardKernel(modules);
                }
            }
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            IocKernel.Initialize(new IocConfiguration());

            base.OnStartup(e);
        }

        App()
        {
            // Register services
            Ioc.Default.ConfigureServices(
                new ServiceCollection()
                    .AddSingleton<ISettingsService, SettingsService>()
                    .AddTransient<HomePageViewModel>() //ViewModels
                    .BuildServiceProvider()
            );

            //new Thread(() =>
            //{
            //    Thread.CurrentThread.IsBackground = false;
            //    /* run your code here */
            //    int i = 0;
            //    while (true)
            //    {
            //        Thread.Sleep(2000);
            //        i++;
            //        Console.WriteLine($"Hello, world{i}");
            //    }

            //}).Start();
        }

        // Single Instance
        /// <summary>The event mutex name.</summary>
        private const string UniqueEventName = "{elaphureLink.wpf.uniqueEvent}";

        /// <summary>The unique mutex name.</summary>
        private const string UniqueMutexName = "{elaphureLink.wpf.uniqueMutex}";

        /// <summary>The event wait handle.</summary>
        private EventWaitHandle eventWaitHandle;

        /// <summary>The mutex.</summary>
        private Mutex mutex;

        /// <summary>The app on startup.</summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The e.</param>
        private void AppOnStartup(object sender, StartupEventArgs e)
        {
            bool isOwned;
            this.mutex = new Mutex(true, UniqueMutexName, out isOwned);
            this.eventWaitHandle = new EventWaitHandle(
                false,
                EventResetMode.AutoReset,
                UniqueEventName
            );

            // So, R# would not give a warning that this variable is not used.
            GC.KeepAlive(this.mutex);

            if (isOwned)
            {
                // Spawn a thread which will be waiting for our event
                var thread = new Thread(() =>
                {
                    while (this.eventWaitHandle.WaitOne())
                    {
                        Current.Dispatcher.BeginInvoke(
                            (Action)(() => ((MainWindow)Current.MainWindow).BringToForeground())
                        );
                    }
                });

                // It is important mark it as background otherwise it will prevent app from exiting.
                thread.IsBackground = true;

                thread.Start();
                return;
            }

            // Notify other instance so it could bring itself to foreground.
            this.eventWaitHandle.Set();

            // Terminate this instance.
            this.Shutdown();
        }
    }
}
