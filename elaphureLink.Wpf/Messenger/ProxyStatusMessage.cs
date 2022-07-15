using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Toolkit.Mvvm.Messaging.Messages;

namespace elaphureLink.Wpf.Messenger
{
    class ProxyStatusChangedMessage : ValueChangedMessage<bool>
    {
        public ProxyStatusChangedMessage(bool value) : base(value)
        {

        }
    }

    public class ProxyStatusRequestMessage : RequestMessage<bool>
    {
    }



}
