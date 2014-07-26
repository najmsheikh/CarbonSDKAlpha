#pragma once

namespace CarbonForge
{
namespace CustomUtilities
{

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for frmSimpleScatter
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class frmSimpleScatter : public System::Windows::Forms::Form
	{
	public:
		frmSimpleScatter( SimpleScatter * utility )
		{
            // Initialize variables to sensible defaults.
            m_pUtility = utility;
            
            // Initialize controls
            InitializeComponent();

            // Select default options.
            comboType->SelectedIndex = 0;

		}

        // Public properties
        property int ObjectType {
            int get() {
                return comboType->SelectedIndex;
            }
        }

        property int ObjectCount {
            int get() {
                return Decimal::ToInt32(numericItems->Value);            }
        }

    private:
        // Private members
        SimpleScatter * m_pUtility;

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~frmSimpleScatter()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;
        private: System::Windows::Forms::ComboBox^  comboType;
        private: System::Windows::Forms::Label^  label1;
        private: System::Windows::Forms::NumericUpDown^  numericItems;
        private: System::Windows::Forms::Label^  label2;
        private: System::Windows::Forms::Button^  buttonOK;
        private: System::Windows::Forms::Button^  buttonCancel;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            this->comboType = (gcnew System::Windows::Forms::ComboBox());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->numericItems = (gcnew System::Windows::Forms::NumericUpDown());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->buttonOK = (gcnew System::Windows::Forms::Button());
            this->buttonCancel = (gcnew System::Windows::Forms::Button());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericItems))->BeginInit();
            this->SuspendLayout();
            // 
            // comboType
            // 
            this->comboType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->comboType->FormattingEnabled = true;
            this->comboType->Items->AddRange(gcnew cli::array< System::Object^  >(3) {L"Cube", L"Sphere", L"Point Light"});
            this->comboType->Location = System::Drawing::Point(75, 12);
            this->comboType->Name = L"comboType";
            this->comboType->Size = System::Drawing::Size(154, 21);
            this->comboType->TabIndex = 0;
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(6, 16);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(65, 13);
            this->label1->TabIndex = 1;
            this->label1->Text = L"Object Type";
            // 
            // numericItems
            // 
            this->numericItems->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, 0});
            this->numericItems->Location = System::Drawing::Point(336, 12);
            this->numericItems->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1000, 0, 0, 0});
            this->numericItems->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 0});
            this->numericItems->Name = L"numericItems";
            this->numericItems->Size = System::Drawing::Size(118, 20);
            this->numericItems->TabIndex = 2;
            this->numericItems->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, 0});
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(235, 16);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(95, 13);
            this->label2->TabIndex = 3;
            this->label2->Text = L"Number of Objects";
            // 
            // buttonOK
            // 
            this->buttonOK->DialogResult = System::Windows::Forms::DialogResult::OK;
            this->buttonOK->Location = System::Drawing::Point(260, 43);
            this->buttonOK->Name = L"buttonOK";
            this->buttonOK->Size = System::Drawing::Size(94, 26);
            this->buttonOK->TabIndex = 4;
            this->buttonOK->Text = L"OK";
            this->buttonOK->UseVisualStyleBackColor = true;
            // 
            // buttonCancel
            // 
            this->buttonCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
            this->buttonCancel->Location = System::Drawing::Point(360, 43);
            this->buttonCancel->Name = L"buttonCancel";
            this->buttonCancel->Size = System::Drawing::Size(94, 26);
            this->buttonCancel->TabIndex = 5;
            this->buttonCancel->Text = L"Cancel";
            this->buttonCancel->UseVisualStyleBackColor = true;
            // 
            // frmSimpleScatter
            // 
            this->AcceptButton = this->buttonOK;
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->CancelButton = this->buttonCancel;
            this->ClientSize = System::Drawing::Size(460, 77);
            this->Controls->Add(this->buttonCancel);
            this->Controls->Add(this->buttonOK);
            this->Controls->Add(this->label2);
            this->Controls->Add(this->numericItems);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->comboType);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"frmSimpleScatter";
            this->Text = L"Simple Object Scatter Example";
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericItems))->EndInit();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion

};
 
} // End Namespace CustomUtilities

} // End Namespace CarbonForge