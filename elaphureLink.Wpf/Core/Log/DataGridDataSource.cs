using NLog.Targets;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace elaphureLink.Wpf.Core.Log
{
    public class DataGridDataSource
    {
        private static ObservableCollection<DataGridDataItem> _items;

        private static CollectionViewSource groupedItems;
        private string _cachedSortedColumn = string.Empty;

        // Loading data
        public async Task<IEnumerable<DataGridDataItem>> GetDataAsync()
        {
            _items = new ObservableCollection<DataGridDataItem>();

            var target = NLog.LogManager.Configuration.FindTargetByName<MemoryTarget>(
                "logMemoryBuffer"
            );
            var logEvents = target.Logs;

            foreach (string logLine in logEvents)
            {
                string[] values = logLine.Split('|');
                _items.Add(
                    new DataGridDataItem()
                    {
                        log_time = values[0],
                        log_level = values[1],
                        log_module = values[2],
                        log_message = values[3]
                    }
                );
            }

            return _items;
        }
    }
}
