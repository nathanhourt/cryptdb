#include "databaseclient.hpp"

#include "crypto.hpp"

DatabaseClient::DatabaseClient(QObject *parent) :
    QObject(parent),
    kPrime(QCA::Random::randomArray(16)),
    kPrimePrime(QCA::Random::randomArray(16)),
    preEncryptIV(QCA::Random::randomArray(16)),
    ks(QCA::Random::randomArray(16)),
    kk(QCA::Random::randomArray(16))
{
}

DB::RowList DatabaseClient::encryptNewRows(DB::RowList newRows, DB::Index nextAvailableIndex)
{
    DB::RowList crypticRows;

    foreach (DB::Row row, newRows) {
        crypticRows.append(DB::Row());
        foreach (DB::Word word, row) {
            word = Crypto::preEncrypt(word, kPrimePrime, preEncryptIV);
            QCA::SecureArray ti = Crypto::generateS(ks, nextAvailableIndex++);
            ti.append(Crypto::generateFki(Crypto::generateKi(kk, word.left(Crypto::N_BYTES - Crypto::M_BYTES)), ti));
            crypticRows.last().append(Crypto::arrayXor(word, ti));
        }
    }

    return crypticRows;
}

DB::Word DatabaseClient::encryptWordForSearch(DB::Word plainText)
{
    Crypto crypt;
    //Split word into X_i
    DB::Word ctxt = crypt.preEncrypt(plainText);
    QCA::SecureArray cipherText(ctxt);
    QCA::SecureArray k = crypt.generateKi(kPrime, cipherText);

    cipherText.append(k);
    return cipherText.toByteArray();
} //end encryptWordForSearch function
