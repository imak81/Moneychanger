
#include <QVBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QGridLayout>
#include <QDateTime>

#include <opentxs/OTAPI.h>
#include <opentxs/OT_ME.h>
#include <opentxs/OTLog.h>

#include "homedetail.h"
#include "ui_homedetail.h"

MTHomeDetail::MTHomeDetail(QWidget *parent) :
    QWidget(parent),
    m_pDetailLayout(NULL),
    ui(new Ui::MTHomeDetail)
{
    ui->setupUi(this);
}

MTHomeDetail::~MTHomeDetail()
{
    // --------------------------------------------------
    if (NULL != m_pDetailLayout)
    {
//      this->clearLayout(m_pDetailLayout);

        // The layout will already be cleared when it gets deleted.
        // Therefore we only use clearLayout when replacing it with
        // a new layout -- we don't have to clear it on destruction.
        // (It already clears itself in that case.)

        delete m_pDetailLayout;
        m_pDetailLayout = NULL;
    }
    // --------------------------------------------------
    delete ui;
}

void MTHomeDetail::clearLayout(QLayout* pLayout)
{
    if ( NULL == pLayout)
        return;
    // -----------------------------------------------
    QLayoutItem * pItemAt = NULL;

    while ( ( pItemAt = pLayout->takeAt( 0 ) ) != NULL )
    {
        if (QWidget * childWidget = pItemAt->widget())
            delete childWidget;
        else if (QLayout* childLayout = pItemAt->layout())
            clearLayout(childLayout);
        delete pItemAt;
    }
}


//static
QWidget * MTHomeDetail::CreateDetailHeaderWidget(MTRecord & recordmt)
{
    // -------------------------------------------
    //Append to transactions list in overview dialog.
    QWidget * row_widget = new QWidget;
    QGridLayout * row_widget_layout = new QGridLayout;

    row_widget_layout->setSpacing(4);
    row_widget_layout->setContentsMargins(10, 4, 10, 4);

    row_widget->setLayout(row_widget_layout);
    row_widget->setStyleSheet("QWidget{background-color:#c0cad4;selection-background-color:#a0aac4;}");
    // -------------------------------------------
    //Render row.
    //Header of row
    QString tx_name = QString(QString::fromStdString(recordmt.GetName()));

    if(tx_name.trimmed() == "")
    {
        //Tx has no name
        tx_name.clear();
        tx_name = "Transaction";
    }

    QLabel * header_of_row = new QLabel;
    QString header_of_row_string = QString("");
    header_of_row_string.append(tx_name);

    header_of_row->setText(header_of_row_string);

    //Append header to layout
    row_widget_layout->addWidget(header_of_row, 0, 0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Amount (with currency tla)
    QLabel * currency_amount_label = new QLabel;
    QString currency_amount = QString("");

    if (recordmt.IsMail())
    {
        if (recordmt.IsOutgoing())
            currency_amount.append("sent message");
        else
            currency_amount.append("message");
    }
    else
    {
        std::string str_formatted;
        QString qstrAmount = QString("");
        if (recordmt.FormatAmount(str_formatted))
            qstrAmount = QString(QString::fromStdString(str_formatted));
        currency_amount.append(qstrAmount);
    }
    // ----------------------------------------------------------------
    long lAmount = OTAPI_Wrap::StringToLong(recordmt.GetAmount());

    if (recordmt.IsOutgoing() || (lAmount < 0))
        currency_amount_label->setStyleSheet("QLabel { color : red; }");
    else
        currency_amount_label->setStyleSheet("QLabel { color : green; }");
    // ----------------------------------------------------------------
    currency_amount_label->setText(currency_amount);
    // ----------------------------------------------------------------
    row_widget_layout->addWidget(currency_amount_label, 0, 1, 1,1, Qt::AlignRight);
    // -------------------------------------------
    //Sub-info
    QWidget * row_content_container = new QWidget;
    QGridLayout * row_content_grid = new QGridLayout;

    // left top right bottom

    row_content_grid->setSpacing(4);
    row_content_grid->setContentsMargins(10, 4, 10, 4);

    row_content_container->setLayout(row_content_grid);

    row_widget_layout->addWidget(row_content_container, 1,0, 1,2);
    // -------------------------------------------
    // Column one
    //Date (sub-info)
    //Calc/convert date/times
    QDateTime timestamp;

    long lDate = OTAPI_Wrap::StringToLong(recordmt.GetDate());

    timestamp.setTime_t(lDate);

    QLabel * row_content_date_label = new QLabel;
    QString row_content_date_label_string;
    row_content_date_label_string.append(QString(timestamp.toString(Qt::SystemLocaleShortDate)));
    row_content_date_label->setText(row_content_date_label_string);

    row_content_grid->addWidget(row_content_date_label, 0,0, 1,1, Qt::AlignLeft);
    // -------------------------------------------
    // Column two
    //Status
    QLabel * row_content_status_label = new QLabel;
    QString row_content_status_string;

//      if (record_map["ismail"].toBool())
//         row_content_status_string.append(record_map["shortMail"].toString());
//      else
//          row_content_status_string.append(record_map["formatDescription"].toString());

    std::string formatDescription_holder;
    recordmt.FormatDescription(formatDescription_holder);

    row_content_status_string.append(QString::fromStdString(formatDescription_holder));
    // -------------------------------------------
    //add string to label
    row_content_status_label->setText(row_content_status_string);

    //add to row_content grid
    row_content_grid->addWidget(row_content_status_label, 0,1, 1,1, Qt::AlignRight);
    // -------------------------------------------
    return row_widget;
}


void SetHeight (QPlainTextEdit* edit, int nRows)
{
  QFontMetrics m (edit -> font()) ;
  int RowHeight = m.lineSpacing() ;
  edit -> setFixedHeight  (nRows * RowHeight) ;
}

void MTHomeDetail::refresh(int nRow, MTRecordList & theList)
{
    // --------------------------------------------------
    if (NULL != m_pDetailLayout)
    {
        this->clearLayout(m_pDetailLayout);
        delete m_pDetailLayout;
        m_pDetailLayout = NULL;
    }
    // --------------------------------------------------
    m_pDetailLayout = new QGridLayout;
    m_pDetailLayout->setAlignment(Qt::AlignTop);
    // --------------------------------------------------
    weak_ptr_MTRecord   weakRecord = theList.GetRecord(nRow);
    shared_ptr_MTRecord record     = weakRecord.lock();

    if (weakRecord.expired())
    {
        this->setLayout(m_pDetailLayout);
        return;
    }
    // --------------------------------------------------
    m_record = record;
    MTRecord & recordmt = *record;
    // --------------------------------------------------
    int nCurrentRow = 0;
    int nCurrentColumn = 1;
    // --------------------------------------------------
    // Add the header widget for this detail.
    //
    QWidget * pHeader = MTHomeDetail::CreateDetailHeaderWidget(recordmt);

    if (NULL != pHeader)
    {
        m_pDetailLayout->addWidget(pHeader, nCurrentRow, nCurrentColumn);
        m_pDetailLayout->setAlignment(pHeader, Qt::AlignTop);
        nCurrentRow++;
    }
    // --------------------------------------------------
    QString nymId  = QString(record->GetOtherNymID().c_str());
    QString acctId = QString(record->GetOtherAccountID().c_str());
    // --------------------------------------------------
    if (record->HasMemo())
    {
        QPlainTextEdit *sec = new QPlainTextEdit;

        QString strMemo = QString(record->GetMemo().c_str());

        sec->setPlainText(strMemo);
        sec->setReadOnly(true);

        m_pDetailLayout->addWidget(sec, nCurrentRow, nCurrentColumn);
        m_pDetailLayout->setAlignment(sec, Qt::AlignTop);

        SetHeight (sec, 2);

        nCurrentRow++;
    }
    // --------------------------------------------------
    QString viewDetails = QString("");

    if (record->IsReceipt() || record->IsOutgoing())
        viewDetails = QString("View Recipient Details");
    else
        viewDetails = QString("View Sender Details");
    // --------------------------------------------------
//    SimpleContact *existingContact = [SimpleContact contactWithNymId:nymId andAccountId:acctId];
    bool bExistingContact = false;

    QString contactActionName = bExistingContact ? viewDetails : QString("Add as Contact");
    QPushButton * contactButton = new QPushButton(contactActionName);

    m_pDetailLayout->addWidget(contactButton, nCurrentRow, nCurrentColumn);
    m_pDetailLayout->setAlignment(contactButton, Qt::AlignTop);

    nCurrentRow++;

    // -------------------------------------------


    /*

    NSMutableArray* actions = [NSMutableArray arrayWithObject:[SectionAction actionWithName:actionName icon:nil block:^(UIViewController* vc) {
        ContactEditorViewController *editor = [[ContactEditorViewController alloc] initWithNibName:nil bundle:nil];
        SimpleContact *cnt = existingContact;
        if (!cnt) {
          cnt = [SimpleContact newContact];
          cnt.nymId = nymId;
          cnt.accountId = acctId;
        }
        editor.contact = cnt;
        self.shouldResetRecordOnView = true; //They can possibly delete the contact on view, so we always have to reset.
        [self.navigationController pushViewController:editor animated:YES];
    }]];
    */



    // --------------------------------------------------
    if (record->CanDeleteRecord())
    {
        QString deleteActionName = record->IsMail() ? QString("Archive this Message") : QString("Archive this Record");
        QPushButton * deleteButton = new QPushButton(deleteActionName);

        m_pDetailLayout->addWidget(deleteButton, nCurrentRow, nCurrentColumn);
        m_pDetailLayout->setAlignment(deleteButton, Qt::AlignTop);

        nCurrentRow++;


        /*
        [actions addObject:[SectionAction actionWithName:(record->IsMail() ? @"Archive this Message" : @"Archive this Record") icon:nil block:^(UIViewController* vc)
        {
            bool (^actionBlock)() = ^bool{return false;};

            NSString *msg = @"Deletion Failed.";
            UIAlertView *fail = [[UIAlertView alloc] initWithTitle:nil message:msg delegate:nil
                                                 cancelButtonTitle:@"OK" otherButtonTitles:nil];

            actionBlock = ^bool {
                bool bSuccess = record->DeleteRecord();

                if (bSuccess)
                {
                    self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                    [self.navigationController popViewControllerAnimated:YES];
                }
                return bSuccess;
            };
            [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:@"Archiving..." withFailureAlert:fail];
        }]];
        */
    }
    // --------------------------------------------------

    if (record->CanAcceptIncoming())
    {
        const bool bIsTransfer = (record->GetRecordType() == MTRecord::Transfer);
        const bool bIsReceipt  = (record->GetRecordType() == MTRecord::Receipt);

        QString nameString;
        QString actionString;

        if (record->IsTransfer())
        {
            nameString = QString("Accept this Transfer");
            actionString = QString("Accepting...");
        }
        else if (record->IsReceipt())
        {
            nameString = QString("Accept this Receipt");
            actionString = QString("Accepting...");
        }
        else if (record->IsInvoice())
        {
            nameString = QString("Pay this Invoice");
            actionString = QString("Paying...");
        }
        else if (record->IsPaymentPlan())
        {
            nameString = QString("Activate this Payment Plan");
            actionString = QString("Activating...");
        }
        else if (record->IsContract())
        {
            nameString = QString("Sign this Smart Contract");
            actionString = QString("Signing...");
        }
        else if (record->IsCash())
        {
            nameString = QString("Deposit this Cash");
            actionString = QString("Depositing...");
        }
        else if (record->IsCheque())
        {
            nameString = QString("Deposit this Cheque");
            actionString = QString("Depositing...");
        }
        else if (record->IsVoucher())
        {
            nameString = QString("Accept this Payment");
            actionString = QString("Accepting...");
        }
        else
        {
            nameString = QString("Deposit this Payment");
            actionString = QString("Depositing...");
        }

        QPushButton * acceptButton = new QPushButton(nameString);

        m_pDetailLayout->addWidget(acceptButton, nCurrentRow, nCurrentColumn);
        m_pDetailLayout->setAlignment(acceptButton, Qt::AlignTop);

        nCurrentRow++;


        /*
        [actions addObject:[SectionAction actionWithName:nameString icon:nil block:^(UIViewController* vc)
        {
            bool (^actionBlock)() = ^bool{return false;};

            NSString *msg = QString("Transaction Failed.");
            UIAlertView *fail = [[UIAlertView alloc] initWithTitle:nil message:msg delegate:nil
                                                 cancelButtonTitle:QString("OK") otherButtonTitles:nil];

            if (bIsTransfer)
            {
                actionBlock = ^bool {
                    bool bSuccess = record->AcceptIncomingTransfer();

                    if (bSuccess)
                    {
                        self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                        [self.navigationController popViewControllerAnimated:YES];
                    }
                    return bSuccess;
                };
                [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];
            }
            else if (bIsReceipt)
            {
                actionBlock = ^bool {
                    bool bSuccess = record->AcceptIncomingReceipt();

                    if (bSuccess)
                    {
                        self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                        [self.navigationController popViewControllerAnimated:YES];
                    }
                    return bSuccess;
                };
                [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];
            }
            else if (record->GetRecordType() == MTRecord::Instrument) {  // TODO: filter these accounts by serverID and asset type ID.
                ContractPickerViewController *picker = [ContractPickerViewController pickerWithTitle:QString("Account")
                                                                                          dataSource:[AccountContractList instance]];
                picker.cancelButtonEnabled = YES;
                picker.callback = ^(ContractPickerViewController* vc, id<ContractWrapper> contract) {

                    bool (^actionBlock)() = ^bool{return false;};

                    if (contract) {
                        actionBlock = ^bool {
                            bool bSuccess = record->AcceptIncomingInstrument(contract.contractId.UTF8String);

                            if (bSuccess)
                            {
                                self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                                [self.navigationController popViewControllerAnimated:YES];
                            }
                            return bSuccess;
                        };
                        [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];
                    }
                    [vc dismissModalViewControllerAnimated:YES];
                };
                [vc presentModalViewController:picker animated:YES];
            }
            //TODO do a refresh vs a pop

        }]];
        */
    }

    if (record->CanCancelOutgoing())
    {
        QString msg = QString("Cancellation Failed. Perhaps recipient had already accepted it?");
//        UIAlertView *fail = [[UIAlertView alloc] initWithTitle:nil message:msg delegate:nil
//                                             cancelButtonTitle:QString("OK") otherButtonTitles:nil];

        QString cancelString;
        QString actionString = QString("Canceling...");

        if (record->IsInvoice())
            cancelString = QString("Cancel this Invoice");
        else if (record->IsPaymentPlan())
            cancelString = QString("Cancel this Payment Plan");
        else if (record->IsContract())
            cancelString = QString("Cancel this Smart Contract");
        else if (record->IsCash())
        {
            cancelString = QString("Recover this Cash");
            actionString = QString("Recovering...");
            msg = QString("Recovery Failed. Perhaps recipient had already accepted it?");
        }
        else if (record->IsCheque())
            cancelString = QString("Cancel this Cheque");
        else if (record->IsVoucher())
            cancelString = QString("Cancel this Payment");
        else
            cancelString = QString("Cancel this Payment");


        QPushButton * cancelButton = new QPushButton(cancelString);

        m_pDetailLayout->addWidget(cancelButton, nCurrentRow, nCurrentColumn);
        m_pDetailLayout->setAlignment(cancelButton, Qt::AlignTop);

        nCurrentRow++;


        /*
        [actions addObject:[SectionAction actionWithName:cancelString icon:nil block:^(UIViewController* vc) {
            bool (^actionBlock)() = ^bool{return false;};

            actionBlock = ^bool {

                if (record->IsCash())
                {
                    ContractPickerViewController *picker = [ContractPickerViewController pickerWithTitle:QString("Account") // TODO: filter these accounts by serverID and asset type ID.
                                                                                              dataSource:[AccountContractList instance]];
                    picker.cancelButtonEnabled = YES;
                    picker.callback = ^(ContractPickerViewController* vc, id<ContractWrapper> contract) {

                        bool (^actionBlock)() = ^bool{return false;};

                        if (contract) {
                            actionBlock = ^bool {
                                OT_ME madeEasy;
                                bool bInnerSuccess = false;
                                if (1 == madeEasy.deposit_cash(record->GetServerID(), record->GetNymID(), contract.contractId.UTF8String, record->GetContents()))
                                {
                                    bInnerSuccess = true;
                                    record->DiscardOutgoingCash();
                                    self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                                    [self.navigationController popViewControllerAnimated:YES];
                                }
                                return bInnerSuccess;
                            };
                            [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];
                        }
                        [vc dismissModalViewControllerAnimated:YES];
                    };
                    [vc presentModalViewController:picker animated:YES];
                }
                else
                {
                    bool bSuccess = record->CancelOutgoing(record->GetAccountID());

                    if (bSuccess)
                    {
                        self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
                        [self.navigationController popViewControllerAnimated:YES];
                    }
                    return bSuccess;
                }

                return true;
            };

            [LoadingView fallibleReloadAction:actionBlock inView:self.view withText:actionString withFailureAlert:fail];

        }]];
        */
    }

    if (record->CanDiscardOutgoingCash())
    {
        QString discardString = QString("Discard this Sent Cash");

        QPushButton * discardButton = new QPushButton(discardString);

        m_pDetailLayout->addWidget(discardButton, nCurrentRow, nCurrentColumn);
        m_pDetailLayout->setAlignment(discardButton, Qt::AlignTop);

        nCurrentRow++;


        /*
        [actions addObject:[SectionAction actionWithName:discardString icon:nil block:^(UIViewController* vc) {
            record->DiscardOutgoingCash();

            self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
            [self.navigationController popViewControllerAnimated:YES];
        }]];
        */
    }

    if (record->CanDiscardIncoming())
    {
        QString discardString;

        if (record->IsInvoice())
            discardString = QString("Discard this Invoice");
        else if (record->IsPaymentPlan())
            discardString = QString("Discard this Payment Plan");
        else if (record->IsContract())
            discardString = QString("Discard this Smart Contract");
        else if (record->IsCash())
            discardString = QString("Discard this Cash");
        else if (record->IsCheque())
            discardString = QString("Discard this Cheque");
        else if (record->IsVoucher())
            discardString = QString("Discard this Payment");
        else
            discardString = QString("Discard this Payment");


        QPushButton * discardButton = new QPushButton(discardString);

        m_pDetailLayout->addWidget(discardButton, nCurrentRow, nCurrentColumn);
        m_pDetailLayout->setAlignment(discardButton, Qt::AlignTop);

        nCurrentRow++;


        /*
        [actions addObject:[SectionAction actionWithName:discardString icon:nil block:^(UIViewController* vc) {
            record->DiscardIncoming();

            self.navigationController.navigationItem.rightBarButtonItem.tintColor = [UIColor redColor];
            [self.navigationController popViewControllerAnimated:YES];
        }]];
        */
    }

    if (!(record->GetOtherNymID().empty()))
    {
        QString msgUser;

        if (record->IsReceipt() || record->IsOutgoing())
            msgUser = QString("Message the Recipient");
        else
            msgUser = QString("Message the Sender");


        QPushButton * msgButton = new QPushButton(((record->IsMail() && !record->IsOutgoing()) ? QString("Reply to this Message") : msgUser));

        m_pDetailLayout->addWidget(msgButton, nCurrentRow, nCurrentColumn);
        m_pDetailLayout->setAlignment(msgButton, Qt::AlignTop);

        nCurrentRow++;


        /*
            // Display the SendMailViewController here.
            //
            SendMailViewController *send_mail_control = [[SendMailViewController alloc] initWithNibName:nil bundle:nil];

            // Pre-fill the recipient here.
//            send_mail_control.record = record;
            send_mail_control.nymID = QString(record->GetOtherNymID().c_str());


            // NOTE: You will need to lookup the CONTACT for the appropriate recipient,
            // since the sendMail page uses a Contact for a recipient.
            // (And what if there IS no contact for that Nym??)

            // Use this call to get the Nym Name:
            //std::string MTNameLookupIPhone::GetNymName(const std::string & str_id) const

            // But what if I don't need the Nym Name? What if I need the actual Contact?

            // Anyway, once you are able to lookup the contact for the appropriate recipient,
            // that means you should also be able to lookup the contact when displaying any
            // transaction detail, so you can remove the "Add as contact" button IN CASES
            // WHERE THE CONTACT ALREADY EXISTS.

            [self.navigationController pushViewController:send_mail_control animated:YES];
        }]];
        */
    }
    // ----------------------------------
    // TRANSACTION IDs DISPLAYED HERE

//    ActionSection *act = [ActionSection sectionWithName:QString("Actions") andActions:actions];
//    act.defaultAlignment = (UITextAlignment) UITextAlignmentLeft;

//    TransactionIdSection* idSec = [TransactionIdSection sectionWithRecord:record];
    // ----------------------------------
//    NSMutableArray *sections = [NSMutableArray arrayWithArray:QString()[
//                                [TransactionSummarySection sectionWithRecord:record],
//                                act,
//                                idSec,
//                                ]];
    // ----------------------------------

    if (record->HasContents())
    {
        QPlainTextEdit *sec = new QPlainTextEdit;

        QString strContents = QString(record->GetContents().c_str());

        sec->setPlainText(strContents);
        sec->setReadOnly(true);

//        if (record->IsMail())
//        {
//            m_pDetailLayout->insertWidget(2, sec, 1); // So the mail contents appear above all the IDs.
//        }
//        else
//        {
            m_pDetailLayout->addWidget(sec, nCurrentRow, nCurrentColumn);
            m_pDetailLayout->setAlignment(sec, Qt::AlignTop);

            nCurrentRow++;

//        }
    }

    // ----------------------------------
/*
    if (record->IsPaymentPlan())
    {
        if (record->HasInitialPayment() || record->HasPaymentPlan())
        {
            std::string str_asset_name = OTAPI_Wrap::GetAssetType_Name(record->GetAssetID().c_str());
            // ---------------------------------
            std::stringstream sss;
            sss << "Payments use the currency: " << str_asset_name << "\n";
            // ---------------------------------
            NSDateFormatter * formatter  = nil;
            NSString        * dateString = nil;
            NSDate          * date       = nil;

            formatter = [[NSDateFormatter alloc] init];
            [formatter setDateFormat:QString("dd-MM-yyyy HH:mm")];

            if (record->HasInitialPayment())
            {
                date = [NSDate dateWithTimeIntervalSince1970:record->GetInitialPaymentDate()];
                dateString = [formatter stringFromDate:[NSDate date]];

                long        lAmount    = record->GetInitialPaymentAmount();
                std::string str_output = OTAPI_Wrap::It()->FormatAmount(record->GetAssetID().c_str(),
                                                                        static_cast<int64_t>(lAmount));
                sss << "Initial payment of " << str_output << " due: " << dateString.UTF8String << "\n";
            }
            // -----------------------------------------------
            if (record->HasPaymentPlan())
            {
                // ----------------------------------------------------------------
                date = [NSDate dateWithTimeIntervalSince1970:record->GetPaymentPlanStartDate()];
                dateString = [formatter stringFromDate:[NSDate date]];

                long        lAmount    = record->GetPaymentPlanAmount();
                std::string str_output = OTAPI_Wrap::It()->FormatAmount(record->GetAssetID().c_str(),
                                                                        static_cast<int64_t>(lAmount));
                sss << "Recurring payments of " << str_output << " begin: " << dateString.UTF8String << " ";
                // ----------------------------------------------------------------
                NSDate *date2 = [NSDate dateWithTimeIntervalSince1970:(record->GetPaymentPlanStartDate() + record->GetTimeBetweenPayments())];
                NSCalendar *calendar = [NSCalendar currentCalendar];
                NSUInteger calendarUnits = NSDayCalendarUnit;
                NSDateComponents *dateComponents = [calendar components:calendarUnits fromDate:date toDate:date2 options:0];

                std::string str_regular = (([dateComponents day] == 1) ?
                                           "and repeat daily." :
                                           "and repeat every %i days.");
                // -----------------------------------------------------------------
                NSString * strInterval = nil;

                if ([dateComponents day] == 1)
                    strInterval = QString(str_regular.c_str());
                else
                    strInterval = [NSString stringWithFormat:QString(str_regular.c_str()),[dateComponents day]];
                // -----------------------------------------------------------------
                sss << strInterval.UTF8String << "\n";
                // -----------------------------------------------------------------
                if (record->GetMaximumNoPayments() > 0)
                    sss << "The maximum number of payments is: " << record->GetMaximumNoPayments() << "\n";
                // -----------------------------------------------------------------
//todo:         inline const time_t &	GetPaymentPlanLength()	 const	{ return m_tPaymentPlanLength; }
            }
            // -----------------------------------------------
            std::string str_details(sss.str());
            TextViewSection *sec = [TextViewSection sectionWithName:QString("Recurring Payment Terms")
                                                      initialString:QString(str_details.c_str()) andStringAction:nil];
            sec.textView.editable = NO;
            sec.customHeight = 80;

            [sections insertObject:sec atIndex:1]; // So the recurring payment terms appears just under the memo.
        }
    }
*/
    // -----------------------------------------------


    this->setLayout(m_pDetailLayout);
}