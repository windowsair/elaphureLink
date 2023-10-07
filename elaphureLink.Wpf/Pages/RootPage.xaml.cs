using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using ModernWpf;
using ModernWpf.Controls;
using ModernWpf.Media.Animation;
using System.Threading;
using System.Globalization;

namespace elaphureLink.Wpf.Pages
{
    public partial class RootPage : UserControl
    {
        private readonly Dictionary<string, Type> pageMap = new Dictionary<string, Type>();
        private readonly NavigationTransitionInfo transitionInfo =
            new DrillInNavigationTransitionInfo();

        public RootPage()
        {
            void AddType(Type t) => this.pageMap.Add(t.Name, t);
            AddType(typeof(InfoPage));
            AddType(typeof(HomePage));
            AddType(typeof(LogPage));

            this.InitializeComponent();
        }

        private void MainFrame_Navigated(
            object sender,
            System.Windows.Navigation.NavigationEventArgs e
        )
        {
            try
            {
                if (sender is ModernWpf.Controls.Frame frame)
                {
                    // We always cache the HomePage so that it is at the top of the stack
                    if (frame.BackStackDepth > 1)
                    {
                        // For other pages, always create new instances and remove them when they are duplicated
                        frame.RemoveBackEntry();
                    }
                }
            }
            catch (Exception) { }
        }

        private void ThemeSwitchOnClick(object sender, RoutedEventArgs e)
        {
            ThemeManager.Current.ApplicationTheme =
                ThemeManager.Current.ActualApplicationTheme == ApplicationTheme.Dark
                    ? ApplicationTheme.Light
                    : ApplicationTheme.Dark;
        }

        private void NavigationViewOnChange(
            NavigationView sender,
            NavigationViewSelectionChangedEventArgs args
        )
        {
            var selectedItem = (NavigationViewItem)args.SelectedItem;
            string pageTag = (string)selectedItem.Tag;
            if (this.pageMap.ContainsKey(pageTag))
            {
                var pageType = this.pageMap[pageTag];

                // JournalEntry firstItem = null;
                // foreach (JournalEntry item in this.MainFrame.BackStack)
                // {
                //     firstItem = item;
                //     continue;
                // }

                // We always cache the HomePage so that it is at the top of the stack
                if (this.MainFrame.BackStackDepth == 1 && pageType.Name == "HomePage")
                {
                    this.MainFrame.GoBack(this.transitionInfo);
                    return;
                }

                // Navigate operation always create a new page instance, our datagrid will lose
                this.MainFrame.Navigate(pageType, null, this.transitionInfo);
            }
        }

        private void LanguageItem_Click(object sender, RoutedEventArgs e)
        {
            // Cast the sender as a MenuItem
            if (sender is MenuItem item)
            {
                // Retrieve the CommandParameter
                var language = item.CommandParameter as string;

                // Handle the selected language
                switch (language)
                {
                    case "fr-fr":
                    case "zh-cn":
                        // Handle language selected
                        Thread.CurrentThread.CurrentCulture = new CultureInfo(language);
                        Thread.CurrentThread.CurrentUICulture = new CultureInfo(language);
                        break;
                    default:
                        // Use the default .resx file which has no localizatition extention (e.g X.fr-FR.resx)
                        Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
                        Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;
                        break;
                }

                // Refresh the main window (or any other windows）
                Application.Current.MainWindow.Content = new RootPage();
            }
        }
    }
}
