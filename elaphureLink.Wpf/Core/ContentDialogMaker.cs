using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ModernWpf.Controls;

namespace elaphureLink.Wpf.Core
{
    // Only a single ContentDialog can be open at any time.
    public static class ContentDialogMaker
    {
        public static async void CreateContentDialog(ContentDialog Dialog, bool awaitPreviousDialog)
        {
            await CreateDialog(Dialog, awaitPreviousDialog);
        }

        public static async Task<ContentDialogResult> CreateContentDialogAsync(
            ContentDialog Dialog,
            bool awaitPreviousDialog
        )
        {
            var res = await CreateDialog(Dialog, awaitPreviousDialog);
            return res;
        }

        static async Task<ContentDialogResult> CreateDialog(
            ContentDialog Dialog,
            bool awaitPreviousDialog
        )
        {
            if (ActiveDialog != null)
            {
                if (awaitPreviousDialog)
                {
                    await DialogAwaiter.Task;
                    DialogAwaiter = new TaskCompletionSource<bool>();
                }
                else
                    ActiveDialog.Hide();
            }
            ActiveDialog = Dialog;
            //ActiveDialog.Closed += ActiveDialog_Closed;
            var res = await ActiveDialog.ShowAsync();
            //ActiveDialog.Closed -= ActiveDialog_Closed;
            return res;
        }

        public static ContentDialog ActiveDialog;
        static TaskCompletionSource<bool> DialogAwaiter = new TaskCompletionSource<bool>();
        //private static void ActiveDialog_Closed(ContentDialog sender, ContentDialogClosedEventArgs args) { DialogAwaiter.SetResult(true); }
    }
}
