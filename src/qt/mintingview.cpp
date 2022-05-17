// Copyright (c) 2012-2022 The BITSZ developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <qt/mintingview.h>
#include <qt/mintingfilterproxy.h>
#include <qt/transactionrecord.h>
#include <qt/mintingtablemodel.h>
#include <qt/walletmodel.h>
#include <qt/guiconstants.h>
#include <qt/guiutil.h>
#include <qt/csvmodelwriter.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>
#include <QScrollBar>
#include <QTableView>
#include <QVBoxLayout>

StakingView::StakingView(QWidget *parent) :
    QWidget(parent), model(0), mintingView(0)
{
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0,0,0,0);

    QString legendBoxStyle = "background-color: rgb(%1,%2,%3); border: 1px solid black;";

    QLabel *youngColor = new QLabel(" ");
    youngColor->setMaximumHeight(15);
    youngColor->setMaximumWidth(10);
    youngColor->setStyleSheet(legendBoxStyle.arg(COLOR_MINT_YOUNG.red()).arg(COLOR_MINT_YOUNG.green()).arg(COLOR_MINT_YOUNG.blue()));
    QLabel *youngLegend = new QLabel(tr("transaction is too young"));
    youngLegend->setContentsMargins(5,0,15,0);

    QLabel *matureColor = new QLabel(" ");
    matureColor->setMaximumHeight(15);
    matureColor->setMaximumWidth(10);
    matureColor->setStyleSheet(legendBoxStyle.arg(COLOR_MINT_MATURE.red()).arg(COLOR_MINT_MATURE.green()).arg(COLOR_MINT_MATURE.blue()));
    QLabel *matureLegend = new QLabel(tr("transaction is mature"));
    matureLegend->setContentsMargins(5,0,15,0);

    QLabel *oldColor = new QLabel(" ");
    oldColor->setMaximumHeight(15);
    oldColor->setMaximumWidth(10);
    oldColor->setStyleSheet(legendBoxStyle.arg(COLOR_MINT_OLD.red()).arg(COLOR_MINT_OLD.green()).arg(COLOR_MINT_OLD.blue()));
    QLabel *oldLegend = new QLabel(tr("transaction has reached maximum probability"));
    oldLegend->setContentsMargins(5,0,15,0);

    QHBoxLayout *legendLayout = new QHBoxLayout();
    legendLayout->setContentsMargins(10,10,0,0);
    legendLayout->addWidget(youngColor);
    legendLayout->addWidget(youngLegend);
    legendLayout->addWidget(matureColor);
    legendLayout->addWidget(matureLegend);
    legendLayout->addWidget(oldColor);
    legendLayout->addWidget(oldLegend);
    legendLayout->insertStretch(-1);

    QLabel *mintingLabel = new QLabel(tr("Display minting probability within : "));
    mintingCombo = new QComboBox();
    mintingCombo->addItem(tr("10 min"), Staking10min);
    mintingCombo->addItem(tr("24 hours"), Staking1day);
    mintingCombo->addItem(tr("30 days"), Staking30days);
    mintingCombo->addItem(tr("90 days"), Staking90days);
    mintingCombo->setFixedWidth(120);
    mintingCombo->setCurrentIndex(1);

    hlayout->insertStretch(0);
    hlayout->addWidget(mintingLabel);
    hlayout->addWidget(mintingCombo);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0,0,0,0);
    vlayout->setSpacing(0);

    QTableView *view = new QTableView(this);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(view);
    vlayout->addLayout(legendLayout);

    vlayout->setSpacing(0);
    int width = view->verticalScrollBar()->sizeHint().width();
    // Cover scroll bar width with spacing
#ifdef Q_WS_MAC
    hlayout->addSpacing(width+2);
#else
    hlayout->addSpacing(width);
#endif
    // Always show scroll bar
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setTabKeyNavigation(false);
    view->setContextMenuPolicy(Qt::CustomContextMenu);

    mintingView = view;

    QAction *copyAddressAction = new QAction(tr("Copy address"), this);
    QAction *copyTransactionIdAction = new QAction(tr("Copy transaction id"), this);

    contextMenu =new QMenu();
    contextMenu->addAction(copyAddressAction);
    contextMenu->addAction(copyTransactionIdAction);

    connect(mintingCombo, SIGNAL(activated(int)), this, SLOT(chooseStakingInterval(int)));
    connect(view, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextualMenu(QPoint)));
    connect(copyAddressAction, SIGNAL(triggered()), this, SLOT(copyAddress()));
    connect(copyTransactionIdAction, SIGNAL(triggered()), this, SLOT(copyTransactionId()));
}


void StakingView::setModel(WalletModel *model)
{
    this->model = model;
    if(model)
    {
        mintingProxyModel = new StakingFilterProxy(this);
        mintingProxyModel->setSourceModel(model->getStakingTableModel());
        mintingProxyModel->setDynamicSortFilter(true);
        mintingProxyModel->setSortRole(Qt::EditRole);
        model->getStakingTableModel()->setStakingProxyModel(mintingProxyModel);

        mintingView->setModel(mintingProxyModel);
        mintingView->setAlternatingRowColors(true);
        mintingView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mintingView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        mintingView->setSortingEnabled(true);
        mintingView->sortByColumn(StakingTableModel::CoinDay, Qt::DescendingOrder);
        mintingView->verticalHeader()->hide();

        mintingView->horizontalHeader()->resizeSection(
                StakingTableModel::Address, 300);
#if QT_VERSION < 0x050000
        mintingView->horizontalHeader()->setResizeMode(
                StakingTableModel::TxHash, QHeaderView::Stretch);
#else
        mintingView->horizontalHeader()->setSectionResizeMode(
                StakingTableModel::TxHash, QHeaderView::Stretch);
#endif

        mintingView->horizontalHeader()->resizeSection(
                StakingTableModel::Age, 60);
        mintingView->horizontalHeader()->resizeSection(
                StakingTableModel::Balance, 100);
        mintingView->horizontalHeader()->resizeSection(
                StakingTableModel::CoinDay,100);
        mintingView->horizontalHeader()->resizeSection(
                StakingTableModel::MintProbability, 120);
    }
}

void StakingView::chooseStakingInterval(int idx)
{
    int interval = 10;
    switch(mintingCombo->itemData(idx).toInt())
    {
        case Staking10min:
            interval = 10;
            break;
        case Staking1day:
            interval = 60*24;
            break;
        case Staking30days:
            interval = 60*24*30;
            break;
        case Staking90days:
            interval = 60*24*90;
            break;
    }
    model->getStakingTableModel()->setStakingInterval(interval);
    mintingProxyModel->invalidate();
}

void StakingView::exportClicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(
            this,
            tr("Export Staking Data"), QString(),
            tr("Comma separated file (*.csv)"), nullptr);

    if (filename.isNull()) return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(mintingProxyModel);
    writer.addColumn(tr("Address"), StakingTableModel::Address);
    writer.addColumn(tr("Transaction"), StakingTableModel::TxHash);
    writer.addColumn(tr("Age"), StakingTableModel::Age);
    writer.addColumn(tr("CoinDay"), StakingTableModel::CoinDay);
    writer.addColumn(tr("Balance"), StakingTableModel::Balance);
    writer.addColumn(tr("StakingProbability"), StakingTableModel::MintProbability);

    if(!writer.write())
    {
        QMessageBox::critical(this, tr("Error exporting"), tr("Could not write to file %1.").arg(filename),
                              QMessageBox::Abort, QMessageBox::Abort);
    }
}

void StakingView::contextualMenu(const QPoint &point)
{
    QModelIndex index = mintingView->indexAt(point);
    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

void StakingView::copyAddress()
{
    GUIUtil::copyEntryData(mintingView, StakingTableModel::Address, Qt::DisplayRole);
}

void StakingView::copyTransactionId()
{
    GUIUtil::copyEntryData(mintingView, StakingTableModel::TxHash, Qt::DisplayRole);
}
