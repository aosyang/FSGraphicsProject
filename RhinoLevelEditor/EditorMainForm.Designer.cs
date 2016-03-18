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
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.btnAddMesh = new System.Windows.Forms.Button();
            this.listMesh = new System.Windows.Forms.ListBox();
            this.engineCanvas1 = new RhinoLevelEditor.EngineCanvas();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Margin = new System.Windows.Forms.Padding(2);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.btnAddMesh);
            this.splitContainer1.Panel1.Controls.Add(this.listMesh);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.engineCanvas1);
            this.splitContainer1.Size = new System.Drawing.Size(812, 562);
            this.splitContainer1.SplitterDistance = 225;
            this.splitContainer1.SplitterWidth = 3;
            this.splitContainer1.TabIndex = 1;
            // 
            // btnAddMesh
            // 
            this.btnAddMesh.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.btnAddMesh.Location = new System.Drawing.Point(2, 534);
            this.btnAddMesh.Margin = new System.Windows.Forms.Padding(2);
            this.btnAddMesh.Name = "btnAddMesh";
            this.btnAddMesh.Size = new System.Drawing.Size(219, 24);
            this.btnAddMesh.TabIndex = 3;
            this.btnAddMesh.Text = "Add to Scene";
            this.btnAddMesh.UseVisualStyleBackColor = true;
            this.btnAddMesh.Click += new System.EventHandler(this.btnAddMesh_Click);
            // 
            // listMesh
            // 
            this.listMesh.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listMesh.FormattingEnabled = true;
            this.listMesh.Location = new System.Drawing.Point(2, 2);
            this.listMesh.Margin = new System.Windows.Forms.Padding(2);
            this.listMesh.Name = "listMesh";
            this.listMesh.Size = new System.Drawing.Size(220, 524);
            this.listMesh.TabIndex = 1;
            // 
            // engineCanvas1
            // 
            this.engineCanvas1.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.engineCanvas1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.engineCanvas1.Location = new System.Drawing.Point(0, 0);
            this.engineCanvas1.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.engineCanvas1.Name = "engineCanvas1";
            this.engineCanvas1.Size = new System.Drawing.Size(582, 560);
            this.engineCanvas1.TabIndex = 1;
            this.engineCanvas1.Load += new System.EventHandler(this.engineCanvas1_Load);
            this.engineCanvas1.KeyDown += new System.Windows.Forms.KeyEventHandler(this.engineCanvas1_KeyDown);
            this.engineCanvas1.KeyUp += new System.Windows.Forms.KeyEventHandler(this.engineCanvas1_KeyUp);
            // 
            // EditorMainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(812, 562);
            this.Controls.Add(this.splitContainer1);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "EditorMainForm";
            this.Text = "Rhino Level Editor";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private EngineCanvas engineCanvas1;
        private System.Windows.Forms.ListBox listMesh;
        private System.Windows.Forms.Button btnAddMesh;
    }
}

