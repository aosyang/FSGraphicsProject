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
            this.splitContainer_AssetView = new System.Windows.Forms.SplitContainer();
            this.splitContainer_EngineCanvas = new System.Windows.Forms.SplitContainer();
            this.btnReplaceMesh = new System.Windows.Forms.Button();
            this.listBox_MeshAssets = new System.Windows.Forms.ListBox();
            this.btnAddMesh = new System.Windows.Forms.Button();
            this.engineCanvas1 = new RhinoLevelEditor.EngineCanvas();
            this.listView_AssetView = new System.Windows.Forms.ListView();
            this.splitContainer_SceneView_and_Property = new System.Windows.Forms.SplitContainer();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.sceneObjectsListBox = new System.Windows.Forms.ListBox();
            this.sceneObjectsSearchTextBox = new System.Windows.Forms.TextBox();
            this.propertyGrid1 = new System.Windows.Forms.PropertyGrid();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteSelectionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.saveMaterialToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.refreshAssetsPreviewsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportAnimToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer_AssetView)).BeginInit();
            this.splitContainer_AssetView.Panel1.SuspendLayout();
            this.splitContainer_AssetView.Panel2.SuspendLayout();
            this.splitContainer_AssetView.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer_EngineCanvas)).BeginInit();
            this.splitContainer_EngineCanvas.Panel1.SuspendLayout();
            this.splitContainer_EngineCanvas.Panel2.SuspendLayout();
            this.splitContainer_EngineCanvas.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer_SceneView_and_Property)).BeginInit();
            this.splitContainer_SceneView_and_Property.Panel1.SuspendLayout();
            this.splitContainer_SceneView_and_Property.Panel2.SuspendLayout();
            this.splitContainer_SceneView_and_Property.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainer1.Location = new System.Drawing.Point(0, 24);
            this.splitContainer1.Margin = new System.Windows.Forms.Padding(2);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.splitContainer_AssetView);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.splitContainer_SceneView_and_Property);
            this.splitContainer1.Size = new System.Drawing.Size(1364, 734);
            this.splitContainer1.SplitterDistance = 1091;
            this.splitContainer1.SplitterWidth = 3;
            this.splitContainer1.TabIndex = 1;
            // 
            // splitContainer_AssetView
            // 
            this.splitContainer_AssetView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer_AssetView.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainer_AssetView.Location = new System.Drawing.Point(0, 0);
            this.splitContainer_AssetView.Name = "splitContainer_AssetView";
            this.splitContainer_AssetView.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer_AssetView.Panel1
            // 
            this.splitContainer_AssetView.Panel1.Controls.Add(this.splitContainer_EngineCanvas);
            // 
            // splitContainer_AssetView.Panel2
            // 
            this.splitContainer_AssetView.Panel2.Controls.Add(this.listView_AssetView);
            this.splitContainer_AssetView.Size = new System.Drawing.Size(1089, 732);
            this.splitContainer_AssetView.SplitterDistance = 509;
            this.splitContainer_AssetView.TabIndex = 1;
            // 
            // splitContainer_EngineCanvas
            // 
            this.splitContainer_EngineCanvas.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.splitContainer_EngineCanvas.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer_EngineCanvas.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splitContainer_EngineCanvas.Location = new System.Drawing.Point(0, 0);
            this.splitContainer_EngineCanvas.Margin = new System.Windows.Forms.Padding(2);
            this.splitContainer_EngineCanvas.Name = "splitContainer_EngineCanvas";
            // 
            // splitContainer_EngineCanvas.Panel1
            // 
            this.splitContainer_EngineCanvas.Panel1.Controls.Add(this.btnReplaceMesh);
            this.splitContainer_EngineCanvas.Panel1.Controls.Add(this.listBox_MeshAssets);
            this.splitContainer_EngineCanvas.Panel1.Controls.Add(this.btnAddMesh);
            // 
            // splitContainer_EngineCanvas.Panel2
            // 
            this.splitContainer_EngineCanvas.Panel2.Controls.Add(this.engineCanvas1);
            this.splitContainer_EngineCanvas.Size = new System.Drawing.Size(1089, 509);
            this.splitContainer_EngineCanvas.SplitterDistance = 218;
            this.splitContainer_EngineCanvas.SplitterWidth = 3;
            this.splitContainer_EngineCanvas.TabIndex = 0;
            // 
            // btnReplaceMesh
            // 
            this.btnReplaceMesh.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.btnReplaceMesh.Location = new System.Drawing.Point(2, 481);
            this.btnReplaceMesh.Margin = new System.Windows.Forms.Padding(2);
            this.btnReplaceMesh.Name = "btnReplaceMesh";
            this.btnReplaceMesh.Size = new System.Drawing.Size(212, 24);
            this.btnReplaceMesh.TabIndex = 4;
            this.btnReplaceMesh.Text = "Replace Selection";
            this.btnReplaceMesh.UseVisualStyleBackColor = true;
            this.btnReplaceMesh.Click += new System.EventHandler(this.btnReplaceMesh_Click);
            // 
            // listBox_MeshAssets
            // 
            this.listBox_MeshAssets.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listBox_MeshAssets.FormattingEnabled = true;
            this.listBox_MeshAssets.Location = new System.Drawing.Point(-1, 0);
            this.listBox_MeshAssets.Margin = new System.Windows.Forms.Padding(2);
            this.listBox_MeshAssets.Name = "listBox_MeshAssets";
            this.listBox_MeshAssets.Size = new System.Drawing.Size(218, 446);
            this.listBox_MeshAssets.TabIndex = 1;
            this.listBox_MeshAssets.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.listMesh_MouseDoubleClick);
            // 
            // btnAddMesh
            // 
            this.btnAddMesh.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.btnAddMesh.Location = new System.Drawing.Point(2, 453);
            this.btnAddMesh.Margin = new System.Windows.Forms.Padding(2);
            this.btnAddMesh.Name = "btnAddMesh";
            this.btnAddMesh.Size = new System.Drawing.Size(212, 24);
            this.btnAddMesh.TabIndex = 3;
            this.btnAddMesh.Text = "Add to Scene";
            this.btnAddMesh.UseVisualStyleBackColor = true;
            this.btnAddMesh.Click += new System.EventHandler(this.btnAddMesh_Click);
            // 
            // engineCanvas1
            // 
            this.engineCanvas1.AutoScroll = true;
            this.engineCanvas1.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.engineCanvas1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.engineCanvas1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.engineCanvas1.Location = new System.Drawing.Point(0, 0);
            this.engineCanvas1.Margin = new System.Windows.Forms.Padding(2, 2, 2, 2);
            this.engineCanvas1.Name = "engineCanvas1";
            this.engineCanvas1.Size = new System.Drawing.Size(866, 507);
            this.engineCanvas1.TabIndex = 2;
            this.engineCanvas1.Load += new System.EventHandler(this.engineCanvas1_Load);
            this.engineCanvas1.KeyDown += new System.Windows.Forms.KeyEventHandler(this.engineCanvas1_KeyDown);
            this.engineCanvas1.KeyUp += new System.Windows.Forms.KeyEventHandler(this.engineCanvas1_KeyUp);
            this.engineCanvas1.MouseClick += new System.Windows.Forms.MouseEventHandler(this.engineCanvas1_MouseClick);
            this.engineCanvas1.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.engineCanvas1_MouseClick);
            // 
            // listView_AssetView
            // 
            this.listView_AssetView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView_AssetView.Location = new System.Drawing.Point(0, 0);
            this.listView_AssetView.Name = "listView_AssetView";
            this.listView_AssetView.Size = new System.Drawing.Size(1089, 219);
            this.listView_AssetView.TabIndex = 0;
            this.listView_AssetView.TileSize = new System.Drawing.Size(256, 256);
            this.listView_AssetView.UseCompatibleStateImageBehavior = false;
            this.listView_AssetView.DoubleClick += new System.EventHandler(this.listView_AssetView_DoubleClick);
            // 
            // splitContainer_SceneView_and_Property
            // 
            this.splitContainer_SceneView_and_Property.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer_SceneView_and_Property.Location = new System.Drawing.Point(0, 0);
            this.splitContainer_SceneView_and_Property.Name = "splitContainer_SceneView_and_Property";
            this.splitContainer_SceneView_and_Property.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer_SceneView_and_Property.Panel1
            // 
            this.splitContainer_SceneView_and_Property.Panel1.Controls.Add(this.groupBox1);
            // 
            // splitContainer_SceneView_and_Property.Panel2
            // 
            this.splitContainer_SceneView_and_Property.Panel2.Controls.Add(this.propertyGrid1);
            this.splitContainer_SceneView_and_Property.Size = new System.Drawing.Size(268, 732);
            this.splitContainer_SceneView_and_Property.SplitterDistance = 366;
            this.splitContainer_SceneView_and_Property.TabIndex = 1;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.sceneObjectsListBox);
            this.groupBox1.Controls.Add(this.sceneObjectsSearchTextBox);
            this.groupBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox1.Location = new System.Drawing.Point(0, 0);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(268, 366);
            this.groupBox1.TabIndex = 2;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Scene View";
            // 
            // sceneObjectsListBox
            // 
            this.sceneObjectsListBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.sceneObjectsListBox.FormattingEnabled = true;
            this.sceneObjectsListBox.Location = new System.Drawing.Point(3, 36);
            this.sceneObjectsListBox.Name = "sceneObjectsListBox";
            this.sceneObjectsListBox.Size = new System.Drawing.Size(262, 327);
            this.sceneObjectsListBox.TabIndex = 1;
            this.sceneObjectsListBox.SelectedIndexChanged += new System.EventHandler(this.sceneObjectsListBox_SelectedIndexChanged);
            // 
            // sceneObjectsSearchTextBox
            // 
            this.sceneObjectsSearchTextBox.Dock = System.Windows.Forms.DockStyle.Top;
            this.sceneObjectsSearchTextBox.Location = new System.Drawing.Point(3, 16);
            this.sceneObjectsSearchTextBox.Name = "sceneObjectsSearchTextBox";
            this.sceneObjectsSearchTextBox.Size = new System.Drawing.Size(262, 20);
            this.sceneObjectsSearchTextBox.TabIndex = 0;
            // 
            // propertyGrid1
            // 
            this.propertyGrid1.CategoryForeColor = System.Drawing.SystemColors.InactiveCaptionText;
            this.propertyGrid1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propertyGrid1.Location = new System.Drawing.Point(0, 0);
            this.propertyGrid1.Margin = new System.Windows.Forms.Padding(2);
            this.propertyGrid1.Name = "propertyGrid1";
            this.propertyGrid1.Size = new System.Drawing.Size(268, 362);
            this.propertyGrid1.TabIndex = 0;
            // 
            // statusStrip1
            // 
            this.statusStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1});
            this.statusStrip1.Location = new System.Drawing.Point(0, 758);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(1364, 22);
            this.statusStrip1.TabIndex = 2;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(0, 17);
            // 
            // menuStrip1
            // 
            this.menuStrip1.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.editToolStripMenuItem,
            this.toolsToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(1364, 24);
            this.menuStrip1.TabIndex = 5;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newToolStripMenuItem,
            this.openToolStripMenuItem,
            this.toolStripSeparator1,
            this.saveToolStripMenuItem,
            this.saveAsToolStripMenuItem,
            this.toolStripSeparator3,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "&File";
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.Size = new System.Drawing.Size(139, 22);
            this.newToolStripMenuItem.Text = "&New map";
            // 
            // openToolStripMenuItem
            // 
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            this.openToolStripMenuItem.Size = new System.Drawing.Size(139, 22);
            this.openToolStripMenuItem.Text = "&Open map...";
            this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(136, 6);
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(139, 22);
            this.saveToolStripMenuItem.Text = "&Save";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
            // 
            // saveAsToolStripMenuItem
            // 
            this.saveAsToolStripMenuItem.Name = "saveAsToolStripMenuItem";
            this.saveAsToolStripMenuItem.Size = new System.Drawing.Size(139, 22);
            this.saveAsToolStripMenuItem.Text = "Save &As...";
            this.saveAsToolStripMenuItem.Click += new System.EventHandler(this.saveAsToolStripMenuItem_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(136, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(139, 22);
            this.exitToolStripMenuItem.Text = "E&xit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.deleteSelectionToolStripMenuItem,
            this.toolStripSeparator2,
            this.saveMaterialToolStripMenuItem,
            this.refreshAssetsPreviewsToolStripMenuItem});
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
            this.editToolStripMenuItem.Text = "&Edit";
            // 
            // deleteSelectionToolStripMenuItem
            // 
            this.deleteSelectionToolStripMenuItem.Name = "deleteSelectionToolStripMenuItem";
            this.deleteSelectionToolStripMenuItem.ShortcutKeys = System.Windows.Forms.Keys.Delete;
            this.deleteSelectionToolStripMenuItem.Size = new System.Drawing.Size(221, 22);
            this.deleteSelectionToolStripMenuItem.Text = "&Delete Selection";
            this.deleteSelectionToolStripMenuItem.Click += new System.EventHandler(this.deleteSelectionToolStripMenuItem_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(218, 6);
            // 
            // saveMaterialToolStripMenuItem
            // 
            this.saveMaterialToolStripMenuItem.Name = "saveMaterialToolStripMenuItem";
            this.saveMaterialToolStripMenuItem.Size = new System.Drawing.Size(221, 22);
            this.saveMaterialToolStripMenuItem.Text = "Save object material to XML";
            this.saveMaterialToolStripMenuItem.Click += new System.EventHandler(this.saveMaterialToolStripMenuItem_Click);
            // 
            // refreshAssetsPreviewsToolStripMenuItem
            // 
            this.refreshAssetsPreviewsToolStripMenuItem.Name = "refreshAssetsPreviewsToolStripMenuItem";
            this.refreshAssetsPreviewsToolStripMenuItem.Size = new System.Drawing.Size(221, 22);
            this.refreshAssetsPreviewsToolStripMenuItem.Text = "Refresh Assets Previews";
            this.refreshAssetsPreviewsToolStripMenuItem.Click += new System.EventHandler(this.refreshAssetsPreviewsToolStripMenuItem_Click);
            // 
            // toolsToolStripMenuItem
            // 
            this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.exportAnimToolStripMenuItem});
            this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
            this.toolsToolStripMenuItem.Size = new System.Drawing.Size(47, 20);
            this.toolsToolStripMenuItem.Text = "&Tools";
            // 
            // exportAnimToolStripMenuItem
            // 
            this.exportAnimToolStripMenuItem.Name = "exportAnimToolStripMenuItem";
            this.exportAnimToolStripMenuItem.Size = new System.Drawing.Size(238, 22);
            this.exportAnimToolStripMenuItem.Text = "&Export fbx animations to binary";
            this.exportAnimToolStripMenuItem.Click += new System.EventHandler(this.exportAnimToolStripMenuItem_Click);
            // 
            // EditorMainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1364, 780);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.menuStrip1);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "EditorMainForm";
            this.Text = "Rhino Level Editor";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.splitContainer_AssetView.Panel1.ResumeLayout(false);
            this.splitContainer_AssetView.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer_AssetView)).EndInit();
            this.splitContainer_AssetView.ResumeLayout(false);
            this.splitContainer_EngineCanvas.Panel1.ResumeLayout(false);
            this.splitContainer_EngineCanvas.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer_EngineCanvas)).EndInit();
            this.splitContainer_EngineCanvas.ResumeLayout(false);
            this.splitContainer_SceneView_and_Property.Panel1.ResumeLayout(false);
            this.splitContainer_SceneView_and_Property.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer_SceneView_and_Property)).EndInit();
            this.splitContainer_SceneView_and_Property.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.Button btnAddMesh;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteSelectionToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem saveAsToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripMenuItem saveMaterialToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exportAnimToolStripMenuItem;
        private System.Windows.Forms.SplitContainer splitContainer_EngineCanvas;
        private EngineCanvas engineCanvas1;
        private System.Windows.Forms.PropertyGrid propertyGrid1;
        private System.Windows.Forms.Button btnReplaceMesh;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.SplitContainer splitContainer_SceneView_and_Property;
        private System.Windows.Forms.ListBox sceneObjectsListBox;
        private System.Windows.Forms.TextBox sceneObjectsSearchTextBox;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.SplitContainer splitContainer_AssetView;
        private System.Windows.Forms.ListView listView_AssetView;
        private System.Windows.Forms.ListBox listBox_MeshAssets;
        private System.Windows.Forms.ToolStripMenuItem refreshAssetsPreviewsToolStripMenuItem;
    }
}

