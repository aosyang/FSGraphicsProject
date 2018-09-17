using System;
using System.Windows.Forms;
using System.IO;
using ManagedInterface;
using System.Reflection;

namespace RhinoLevelEditor
{
    public partial class EngineCanvas : UserControl
    {
        IManagedEngine Engine;
        public IManagedEngine RhinoEngine
        {
            get { return Engine; }
        }

        ToolStripStatusLabel LogLabel;

        public EngineCanvas()
        {
            try
            {
                InitializeComponent();
            }
            catch (FileNotFoundException e)
            {
                MessageBox.Show("Unable to open file: " + e.FileName);
            }
        }

        public void Initialize()
        {
            if (!DesignMode)
            {
                Application.Idle += HandleApplicationIdle;

                Assembly EngineWrapperModule = null;
                try
                {
                    EngineWrapperModule = Assembly.LoadFrom("EngineManagedWrapper.dll");
                }
                catch (FileNotFoundException e)
                {

                }

                if (EngineWrapperModule != null)
                {
                    // Get all types of classes which inherits from IManagedEngine
                    Type[] types = EngineWrapperModule.GetTypes();
                    foreach (Type type in types)
                    {
                        if (!typeof(IManagedEngine).IsAssignableFrom(type))
                        {
                            continue;
                        }

                        Engine = EngineWrapperModule.CreateInstance(type.FullName) as IManagedEngine;
                        if (Engine != null)
                        {
                            Engine.Initialize(Handle);
                            break;
                        }
                    }
                }
            }
        }

        public void SetLogLabel(ref ToolStripStatusLabel logLabel)
        {
            LogLabel = logLabel;
        }

        void HandleApplicationIdle(object sender, EventArgs e)
        {
            if (!DesignMode)
            {
                if (Engine != null)
                {
                    Engine.RunOneFrame();
                }
            }
        }

        public void Shutdown()
        {
            if (!DesignMode)
            {
                Application.Idle -= HandleApplicationIdle;
                if (Engine != null)
                {
                    Engine.Shutdown();
                }
            }
        }

        private void EngineCanvas_Resize(object sender, EventArgs e)
        {
            if (!DesignMode)
            {
                Control control = (Control)sender;
                if (Engine != null)
                {
                    Engine.Resize(control.Width, control.Height);
                }
            }
        }
    }
}
