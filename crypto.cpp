#include "crypto.hpp"
#include <QDebug>

QCA::SecureArray Crypto::generateS(QCA::SecureArray ks, DB::Index i) {
    QCA::Hash hasher("sha1");
    hasher.update(ks);
    hasher.update((char*)&i, sizeof(i));
    QCA::SecureArray result = hasher.final();

    return result.toByteArray().right(N_BYTES - M_BYTES);
}

QCA::SecureArray Crypto::generateKi(QCA::SecureArray kk, QCA::SecureArray Li)
{
    QCA::Hash hasher("sha1");
    hasher.update(kk);
    hasher.update(Li);
    QCA::SecureArray result = hasher.final();

    return result;
}

DB::Word Crypto::preEncrypt(DB::Word wi, QCA::SymmetricKey akey, QCA::InitializationVector iv) {
    QCA::Cipher cipher(QString("aes128"),QCA::Cipher::CBC,
                              // use Default padding, which is equivalent to PKCS7 for CBC
                              QCA::Cipher::DefaultPadding,
                              // this object will encrypt
                              QCA::Encode,
                              akey, iv);
    QCA::SecureArray data(wi);
    qWarning() << "Clear: " << data.toByteArray().toHex();
    QCA::SecureArray encdata = cipher.process(data);

    if(!cipher.ok()) {
        printf("Error\n");
    }
    else {
        printf("Encryption Ok\n");
    }

    wi = encdata.toByteArray();
    return wi;
}

DB::Word Crypto::postDecrypt(DB::Word ctxt, QCA::SecureArray ks, DB::Index i, QCA::SecureArray kk, QCA::InitializationVector iv)
{
    QCA::SecureArray Si = generateS(ks, i);
    QCA::SecureArray Li = arrayXor(Si, QCA::SecureArray(ctxt).toByteArray().right(N_BYTES - M_BYTES));
    QCA::SecureArray ki = generateKi(kk, Li);

    return preEncrypt(ctxt, ki, iv);
} //end postDecrypt function

/*
 * Function returns the result of SHA1(k_i + Si)
 */
QCA::SecureArray Crypto::generateFki(QCA::SecureArray k_i, QCA::SecureArray Si)
{
    QCA::Hash hasher("sha1");
    hasher.update(k_i);
    hasher.update(Si);
    QCA::SecureArray result = hasher.final();

    return result.toByteArray().right(M_BYTES);
}

bool Crypto::clientWordMatchesDatabaseWord(DB::Word clientWord, DB::Word databaseWord, QCA::SecureArray k_i)
{
    QByteArray ti = arrayXor(clientWord, databaseWord).toByteArray();
    return generateFki(k_i, ti.left(N_BYTES - M_BYTES)) == ti.right(M_BYTES);
}

QCA::SecureArray Crypto::arrayXor(QCA::SecureArray a, QCA::SecureArray b)
{
    if (a.size() != b.size())
        return QCA::SecureArray();

    QCA::SecureArray c(a.size());
    for (int i = 0; i < a.size(); ++i)
        c[i] = a[i] ^ b[i];

    return c;
}


const quint32 Crypto::N_BYTES = 4;
const quint32 Crypto::M_BYTES = 1;
