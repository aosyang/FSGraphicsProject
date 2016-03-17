namespace RhinoLevelEditor
{
    partial class EditorMainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.engineCanvas1 = new RhinoLevelEditor.EngineCanvas();
            this.SuspendLayout();
            // 
            // engineCanvas1
            // 
            this.engineCanvas1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.engineCanvas1.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.engineCanvas1.Location = new System.Drawing.Point(12, 12);
            this.engineCanvas1.Name = "engineCanvas1";
            this.engineCanvas1.Size = new System.Drawing.Size(653, 407);
            this.engineCanvas1.TabIndex = 0;
            this.engineCanvas1.Load += new System.EventHandler(this.engineCanvas1_Load);
            // 
            // EditorMainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(677, 431);
            this.Controls.Add(this.engineCanvas1);
            this.Name = "EditorMainForm";
            this.Text = "Rhino Level Editor";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.ResumeLayout(false);

        }

        #endregion

        private EngineCanvas engineCanvas1;
    }
}

