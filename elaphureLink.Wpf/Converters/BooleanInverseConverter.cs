using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace elaphureLink.Wpf.Converters
{

    class BooleanInverseConverter : IValueConverter
    {
        public object Convert(object value,
                                Type targetType,
                                object parameter,
                                System.Globalization.CultureInfo culture)
            => !(bool)value;

        public object ConvertBack(object value,
                                    Type targetType,
                                    object parameter,
                                    System.Globalization.CultureInfo culture)
            => !(bool)value;
    }

}
