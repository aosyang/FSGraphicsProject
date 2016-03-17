using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using EngineManagedWrapper;
using System.IO;

namespace RhinoLevelEditor
{
    public partial class EngineCanvas : UserControl
    {
        RhinoEngineWrapper Engine;

        public EngineCanvas()
        {
            InitializeComponent();
        }

        public void Initialize()
        {
            if (!DesignMode)
            {
                Application.Idle += HandleApplicationIdle;

                Engine = new RhinoEngineWrapper();
                Engine.Initialize(Handle);
            }
        }

        void HandleApplicationIdle(object sender, EventArgs e)
        {
            if (!DesignMode)
            {
                Engine.RunOneFrame();
            }
        }

        public void Shutdown()
        {
            if (!DesignMode)
            {
                Application.Idle -= HandleApplicationIdle;
                Engine.Shutdown();
            }
        }

        private void EngineCanvas_Resize(object sender, EventArgs e)
        {
            if (!DesignMode)
            {
                Control control = (Control)sender;
                if (Engine != null)
                    Engine.Resize(control.Width, control.Height);
            }
        }
    }
}
