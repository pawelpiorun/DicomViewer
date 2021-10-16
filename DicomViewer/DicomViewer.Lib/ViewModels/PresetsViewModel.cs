using DicomViewer.Lib.Common;
using DicomViewer.Lib.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace DicomViewer.Lib.ViewModels
{
    public class PresetsViewModel : NotifyPropertyChanged
    {
        public PresetsViewModel()
        {
            selectedPreset = Presets?.FirstOrDefault();
            OnPropertyChanged(nameof(SelectedPreset));
        }

        public List<Preset> Presets
        {
            get => presets ?? (presets = Helpers.Presets.Instance.GetDefaultPresets());
        }
        List<Preset> presets;

        public Preset SelectedPreset
        {
            get { return selectedPreset; }
            set
            {
                if (selectedPreset == value) return;
                selectedPreset = value;
                OnPropertyChanged();
            }
        }
        Preset selectedPreset;

        internal void SelectPreset(PresetType type)
        {
            var preset = Presets?.FirstOrDefault(x => x.PresetType == type);
            if (preset != null)
                SelectedPreset = preset;
        }
    }
}
