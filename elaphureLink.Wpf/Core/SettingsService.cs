using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace elaphureLink.Wpf.Core.Services
{
    /// <summary>
    /// A simple <see langword="class"/> that handles the local app settings.
    /// </summary>
    public sealed class SettingsService : ISettingsService
    {
        /// <summary>
        /// The <see cref="IPropertySet"/> with the settings targeted by the current instance.
        /// </summary>
        //private readonly IPropertySet SettingsStorage = null;
        //Properties.Settings
        //ApplicationData.Current.LocalSettings.Values;

        public SettingsService()
        {
            // TODO:
        }

        /// <inheritdoc/>
        public void SetValue<T>(string key, T value)
        {
            Properties.Settings.Default[key] = value;
        }

        /// <inheritdoc/>
        public T GetValue<T>(string key)
        {
            return (T)Properties.Settings.Default[key];
        }
    }
}
