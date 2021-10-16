﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DicomViewer.Lib.Interfaces
{
    public interface IWithProperties
    {
        Dictionary<string, string> Properties { get; }
    }
}
