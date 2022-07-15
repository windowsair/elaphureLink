using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using Microsoft.Toolkit.Mvvm;
using Microsoft.Toolkit.Mvvm.ComponentModel;
using Microsoft.Toolkit.Mvvm.Input;

using elaphureLink.Wpf.Core.Services;
using Ninject;


using static elaphureLink.Wpf.App;
using System.Runtime.InteropServices;
using NLog.Targets;

namespace elaphureLink.Wpf.ViewModel
{
    class ViewModelLocator
    {
        public HomePageViewModel HomePageViewModel
        {
            get { return IocKernel.Get<HomePageViewModel>(); } // Loading UserControlViewModel will automatically load the binding for IStorage
        }

        public InfoPageViewModel InfoPageViewModel
        {
            get { return IocKernel.Get<InfoPageViewModel>(); }
        }

    }
}
