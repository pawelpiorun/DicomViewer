﻿using DicomViewer.Lib.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace DicomViewer.UI.Dialogs
{
    public class DialogTemplateSelector : DataTemplateSelector
    {
        public DataTemplate ProgressDialogDataTemplate { get; set; }
        public DataTemplate UnknownDataTemplate { get; set; }

        public override DataTemplate SelectTemplate(object item, DependencyObject container)
        {
            if (item is ProgressViewModel)
                return ProgressDialogDataTemplate;

            return UnknownDataTemplate;
        }
    }
}
