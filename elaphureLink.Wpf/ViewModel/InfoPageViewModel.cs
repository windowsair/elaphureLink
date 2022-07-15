using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Toolkit.Mvvm;
using Microsoft.Toolkit.Mvvm.ComponentModel;
using Microsoft.Toolkit.Mvvm.Input;
using Microsoft.Toolkit.Mvvm.Messaging;

namespace elaphureLink.Wpf.ViewModel
{
    class InfoPageViewModel : ObservableObject
    {
        public InfoPageViewModel()
        {

        }

        public string GitVersionText
        {
            get
            {
                // Tag == git describe command
                return ThisAssembly.Git.Tag + "_" + ThisAssembly.Git.CommitDate;
            }
        }
    }
}
