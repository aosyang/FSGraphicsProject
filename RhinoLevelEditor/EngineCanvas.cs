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
using System.Timers;

namespace RhinoLevelEditor
{
    public partial class EngineCanvas : UserControl
    {
        RhinoEngineWrapper Engine;

        public EngineCanvas()
        {
            InitializeComponent();

            Application.Idle += HandleApplicationIdle;

            Engine = new RhinoEngineWrapper();
            Engine.Initialize(Handle);
        }

        void HandleApplicationIdle(object sender, EventArgs e)
        {
            Engine.RunOneFrame();
        }

        public void Shutdown()
        {
            Application.Idle -= HandleApplicationIdle;
            Engine.Shutdown();
        }

        private void EngineCanvas_Resize(object sender, EventArgs e)
        {
            Control control = (Control)sender;
            if (Engine != null)
                Engine.Resize(control.Width, control.Height);
        }
    }
}
