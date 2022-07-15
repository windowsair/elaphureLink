using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;

namespace elaphureLink.Wpf.Core.Log
{
    public class DataGridDataItem : INotifyDataErrorInfo, IComparable
    {
        private Dictionary<string, List<string>> _errors = new Dictionary<string, List<string>>();

        private string _log_time;
        private string _log_level;
        private string _log_module;
        private string _log_message;

        public string log_time
        {
            get
            {
                return _log_time;
            }

            set
            {
                if (_log_time != value)
                {
                    _log_time = value;
                }
            }
        }

        public string log_level
        {
            get
            {
                return _log_level;
            }

            set
            {
                if (_log_level != value)
                {
                    _log_level = value;
                }
            }
        }

        public string log_module
        {
            get
            {
                return _log_module;
            }

            set
            {
                if (_log_module != value)
                {
                    _log_module = value;
                }
            }
        }

        public string log_message
        {
            get
            {
                return _log_message;
            }

            set
            {
                if (_log_message != value)
                {
                    _log_message = value;
                }
            }
        }

        public event EventHandler<DataErrorsChangedEventArgs> ErrorsChanged;


        public bool CheckBoxColumnValue { get; set; } = true;

        bool INotifyDataErrorInfo.HasErrors
        {
            get
            {
                return _errors.Keys.Count > 0;
            }
        }

        IEnumerable INotifyDataErrorInfo.GetErrors(string propertyName)
        {
            if (propertyName == null)
            {
                propertyName = string.Empty;
            }

            if (_errors.Keys.Contains(propertyName))
            {
                return _errors[propertyName];
            }
            else
            {
                return null;
            }
        }

        int IComparable.CompareTo(object obj)
        {
            int lnCompare = log_time.CompareTo((obj as DataGridDataItem).log_time);

            if (lnCompare == 0)
            {
                return lnCompare;
                //return Parent_mountain.CompareTo((obj as DataGridDataItem).Parent_mountain);
            }
            else
            {
                return lnCompare;
            }
        }
    }

}
