//#
//# Copyright (C) 2018-2021 Yankovich Andrei (EndrII).
//# Distributed under the lgplv3 software license, see the accompanying
//# Everyone is permitted to copy and distribute verbatim copies
//# of this license document, but changing it is not allowed.
//#

#include "countrysparser.h"
#include "lvmainmodel.h"
#include <QXmlStreamReader>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QRegularExpression>

namespace LoginView {


LVMainModel::LVMainModel(const QString modelName, QObject *parent)
    : QObject(parent) {
    m_modelName = modelName;
    m_validLvl = static_cast<PasswordValidationLvl>(
                PasswordValidationLvl::Size8andMore |
                PasswordValidationLvl::LitinSmallChars |
                PasswordValidationLvl::LatinLargeChars);
}

LVMainModel::~LVMainModel() {
    if (m_countrysParser) {
        delete m_countrysParser;
    }
}

bool LVMainModel::setCountriesSource(const QString &path) {

    if (!m_countrysParser) {
        m_countrysParser = new CountrysParser();
    }

    m_countrysParser->parseXMLSource(path, m_countryList);

    emit countryListChanged();
    emit countryCodeListChanged();
    return true;
}

bool LVMainModel::init( QQmlApplicationEngine *engine) {
    if (!engine)
        return false;

    auto root = engine->rootContext();
    if (!root)
        return false;

    if (!root->contextProperty(m_modelName).isNull()) {
        qDebug() << m_modelName << " already used!";
        return false;
    }

    engine->addImportPath(":/");
    root->setContextProperty(m_modelName, this);
    root->setContextProperty("ViewComponents", QVariant::fromValue(ViewComponentsQML{}));

    qRegisterMetaType<LoginView::UserData>("UserData");
    qRegisterMetaType<LoginView::UserViewValidationData>("UserViewValidationData");

    return true;
}

int LVMainModel::country() const {
    return m_country;
}

QStringList LVMainModel::countryList() const {
    return m_countryList.values();
}

UserData LVMainModel::data() const {
    return m_data;
}

UserViewValidationData LVMainModel::validData() const {
    return m_validationData;
}

QString LVMainModel::passwordError() const {
    return m_passwordError;
}

void LVMainModel::setCountry(int country) {
    if (m_country == country)
        return;

    m_country = country;
    emit countryListChanged();
}

void LVMainModel::setData(const UserData& data) {
    if (m_data == data)
        return;

    m_data = data;
    checkValid(data);
    emit dataChanged(m_data);
}

void LVMainModel::setAcceptButtonText(const QString &acceptButtonText) {
    if (m_acceptButtonText == acceptButtonText)
        return;

    m_acceptButtonText = acceptButtonText;
    emit acceptButtonTextChanged(m_acceptButtonText);
}

void LVMainModel::setPasswordError(QString passwordError) {
    if (m_passwordError == passwordError)
        return;

    m_passwordError = passwordError;
    emit passwordErrorChanged(m_passwordError);
}

ViewComponents LVMainModel::components() const {
    return m_components;
}

void LVMainModel::setComponents(const ViewComponents &components) {
    if (m_components != components) {
        m_components = components;
        emit showChanged();
    }
}

bool LVMainModel::fTitle() const {
    return m_components & ViewComponents::Title;
}

bool LVMainModel::fFirstName() const {
    return m_components & ViewComponents::FirstName;
}

bool LVMainModel::fLastName() const {
    return m_components & ViewComponents::LastName;
}

bool LVMainModel::fNickname() const {
    return m_components & ViewComponents::Nickname;
}

bool LVMainModel::fPassword() const {
    return m_components & ViewComponents::Password;
}

bool LVMainModel::fEMail() const {
    return m_components & ViewComponents::EMail;
}

bool LVMainModel::fRegister() const {
    return m_components & ViewComponents::SigupPage;
}

bool LVMainModel::fTermOfUse() const {
    return m_components & ViewComponents::TermOfUse;
}

bool LVMainModel::fLogin() const {
    return m_components & ViewComponents::LoginPage;
}

void LVMainModel::clear() {
    emit clearView();
    setData({});
}

QString LVMainModel::acceptButtonText() const {
    return m_acceptButtonText;
}

void LVMainModel::setCurrentPage(int page) {
    if (page == m_currentPage) {
        return;
    }

    if (!(page & m_components)) {
        return;
    }

    m_currentPage = static_cast<ViewComponents>(page);
    emit currentPageChanged(page);
}

ViewComponents LVMainModel::currentPage() const {
    return m_currentPage;
}

void LVMainModel::setValidData(UserViewValidationData validationData) {
    if (m_validationData == validationData)
        return;

    m_validationData = validationData;

    QString temp;
    if (m_validLvl & PasswordValidationLvl::Size8andMore) {
        temp += tr("must be longer than 8 characters");
    }

    if (m_validLvl & PasswordValidationLvl::NumberChars) {
        temp += ((!temp.isEmpty())? ", ": "") + tr("must contain numbers");
    }

    if (m_validLvl & PasswordValidationLvl::LitinSmallChars) {
        temp += ((!temp.isEmpty())? ", ": "") + tr("must contain small chars");
    }

    if (m_validLvl & PasswordValidationLvl::LatinLargeChars) {
        temp += ((!temp.isEmpty())? ", ": "") + tr("must contain large chars");
    }

    if (m_validLvl & PasswordValidationLvl::ExtraChars) {
        temp += ((!temp.isEmpty())? ", ": "") + tr("must contain extra chars (!@#$%^&*)");
    }

    setPasswordError((temp.isEmpty())? "": tr("Password requirements: ") + temp);

    emit validDataChanged(m_validationData);
}

void LVMainModel::checkValid(const UserData& data) {

    UserViewValidationData result;

    result.setNickname(fNickname() && (data.nickname().contains(" ") || data.nickname().isEmpty()));
    result.setEmail(fEMail() && !data.email().contains(QRegularExpression("^([a-zA-Z0-9_\-\.]+)@([a-zA-Z0-9_\-\.]+)\.([a-zA-Z]{2,5})$")));
    result.setCountry(m_countryList.size() && !m_countryList.contains(data.country()));
    result.setLastName(fLastName() && false);
    result.setFirstName(fFirstName() && (data.firstName().contains(" ") || data.firstName().isEmpty()));

    bool passwordValidation = true;

    if (fPassword()) {
        if (m_validLvl & PasswordValidationLvl::Size8andMore) {
            passwordValidation = passwordValidation &&
                    data.rawPassword().length() >= 8;
        }

        if (m_validLvl & PasswordValidationLvl::NumberChars) {
            passwordValidation = passwordValidation &&
                    data.rawPassword().contains(QRegularExpression("[0-9]"));
        }

        if (m_validLvl & PasswordValidationLvl::LitinSmallChars) {
            passwordValidation = passwordValidation &&
                    data.rawPassword().contains(QRegularExpression("[a-z]"));
        }

        if (m_validLvl & PasswordValidationLvl::LatinLargeChars) {
            passwordValidation = passwordValidation &&
                    data.rawPassword().contains(QRegularExpression("[A-Z]"));
        }

        if (m_validLvl & PasswordValidationLvl::ExtraChars) {
            passwordValidation = passwordValidation &&
                    data.rawPassword().contains(QRegularExpression("[!@#$%^&*]"));
        }

    }

    result.setRawPassword(!passwordValidation);

    setValidData(result);
}

QList<int> LoginView::LVMainModel::countryCodeList() const {
    return m_countryList.keys();
}

void LVMainModel::loginRequest() {
    if (!m_validationData.email()) {
        emit sigLoginRequest(m_data);
    }
}

void LVMainModel::rememberPasswordRequest() {
    if (!m_validationData.email()) {
        emit sigForgotPasswordRequest(m_data);
    }
}

void LVMainModel::registerRequest() {
    if (m_validationData.noError()) {
        emit sigRegisterRequest(m_data);
    }
}

void LVMainModel::showTermOfUseRequest() {
    emit sigShowTermOfUseRequest();
}

}
