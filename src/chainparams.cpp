// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include "chainparamsseeds.h"
#include "arith_uint256.h" //b04902091

// Mine the genesis block if the structure of the block chain is modified.
#ifndef FORCE_MINE_GENESIS
#undef MINE_MAIN_GENESIS
#undef MINE_TESTNET_GENESIS
#undef MINE_REGTEST_GENESIS
#endif


#if defined(MINE_MAIN_GENESIS) || defined(MINE_TESTNET_GENESIS) || defined(MINE_REGTEST_GENESIS)
  #include <stdio.h>
  #include <time.h>
  #include "pow.h"
#endif


//#if undef(MINE_MAIN_GENESIS) || undef(MINE_TESTNET_GENESIS) || undef(MINE_REGTEST_GENESIS)
//#endif

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward, 
				uint32_t nTimeNonce, uint32_t nNonce2, uint32_t nTimeNonce2, uint256 maxhash, uint256 maxhash2, uint256 hashMerkleRoot2)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
/*
printf("\n\nnTime = %u\nnNonce = %u\nnBits = %u\nnVersion = %d\nnTimeNonce = %u\nnNonce2 = %u\nnTimeNonce2 = %u\nmaxhash = %s\nmaxhash2 = %s\nhashMerkleRoot2 = %s\n\n"
, nTime, nNonce, nBits, nVersion, nTimeNonce, nNonce2, nTimeNonce2, maxhash.ToString().c_str(), maxhash2.ToString().c_str(), hashMerkleRoot2.ToString().c_str());
*/
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
//b04902091
    genesis.nTimeNonce = nTimeNonce;
    genesis.maxhash = maxhash;       
    genesis.nNonce2 = nNonce2;               
    genesis.nTimeNonce2 = nTimeNonce2;
    genesis.maxhash2 = maxhash2;
    genesis.hashMerkleRoot2 = hashMerkleRoot2; 
//b04902091
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward,
				uint32_t nTimeNonce, uint32_t nNonce2, uint32_t nTimeNonce2, uint256 maxhash, uint256 maxhash2, uint256 hashMerkleRoot2)
{
    const char* pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward, nTimeNonce, nNonce2, nTimeNonce2, maxhash, maxhash2, hashMerkleRoot2);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
	/*
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 227931;
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.BIP65Height = 388381; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.BIP66Height = 363725; // 00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
        consensus.powLimit = uint256S("0000007fffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
	consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1479168000; // November 15th, 2016.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1510704000; // November 15th, 2017.

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000723d3581fe1bd55373540a");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000003b9ce759c2a087d52abc4266f8f4ebd6d768b89defa50a"); //477890
	*/
	    consensus.nSubsidyHalvingInterval = 210000; // through how many blocks to cut the reward in half
        consensus.BIP34Height = 1; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        //pchMessageStart[0] = 0xf9;
	    pchMessageStart[0] = 0x8C;
        pchMessageStart[1] = 0x70;
        pchMessageStart[2] = 0x5A;
        pchMessageStart[3] = 0xA8;
        nDefaultPort = 8333;
        nPruneAfterHeight = 100000;

#ifdef MINE_MAIN_GENESIS
	uint32_t t = time(NULL);
	uint256 zero;
	zero.SetHex("0");
        fprintf(stderr, "Mining main genesis block...\n\ntime = %u\n", t);
        for (; ; ++t) {
            for (uint32_t n = 0; ; ++n) {
                if ((n & 0xfffff) == 0) fprintf(stderr, "\rnonce = %u", n);
		    uint32_t tn = time(NULL);
		    if(n == 0){
                	genesis = CreateGenesisBlock(t, n, 0x1d7fffff, 1, 50 * COIN, tn, n, tn, zero, zero, zero);
			continue;
		    }
		    else{
			if(UintToArith256(genesis.GetHash()) >= UintToArith256(genesis.maxhash)){
			    genesis = CreateGenesisBlock(t, n, 0x1d7fffff, 1, 50 * COIN, tn, genesis.nNonce, genesis.nTimeNonce, genesis.GetHash(), genesis.maxhash, genesis.hashMerkleRoot);
			}
			else{
			    genesis = CreateGenesisBlock(t, n, 0x1d7fffff, 1, 50 * COIN, tn, genesis.nNonce2, genesis.nTimeNonce2, genesis.maxhash, genesis.maxhash2, genesis.hashMerkleRoot2);
			}
		    }
                if (CheckProofOfWork(genesis.GetHash(), genesis.nBits, consensus)) {
                    fprintf(stderr,
                            "\rnonce = %u, %u\nnTime= %u\nnTimeNonce= %u\nmaxhash = %s,%s\nnNonce2 = %u\nnTimeNonce2 = %u\nmaxhash2 = %s\nhashMerkleRoot2 = %s\nhashMerkleRoot = %s\nhash = %s\n\n",
				n, genesis.nNonce, genesis.nTime, genesis.nTimeNonce, genesis.maxhash.ToString().c_str(), genesis.GetHash2().ToString().c_str(), genesis.nNonce2, genesis.nTimeNonce2,
				genesis.maxhash2.ToString().c_str(),genesis.hashMerkleRoot2.ToString().c_str(),genesis.hashMerkleRoot.ToString().c_str(),genesis.GetHash().ToString().c_str());
                    assert(false);
                }
                if (n == 4294967295) break;
            }
        }
#endif
	/*
	uint256 maxhash, maxhash2, hashMerkleRoot2;
	maxhash.SetHex("ffffffccd7ea595fbbab0c64c3f2c038f06c320c2e241f484e4456996a2d6229");
	maxhash2.SetHex("ffffff7738f9b27737d05ab97cc15d4d7d9a41b8840aea10f86d72d73e469d59");
	hashMerkleRoot2.SetHex("d9f2a49b88a6a667ec31635b1148378d656eab79ba1bd4736cfe51516464980f");
	genesis = CreateGenesisBlock(1664890829, 50745297, 0x1d7fffff, 1, 50 * COIN, 1664892583, 45398641, 1664892399, maxhash, maxhash2, hashMerkleRoot2);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000600de428e4712a31fc81571b47974361201557b4c760e85295d41e44ad"));
        assert(genesis.hashMerkleRoot == uint256S("0xd9f2a49b88a6a667ec31635b1148378d656eab79ba1bd4736cfe51516464980f"));
	*/
	
	    uint256 maxhash, maxhash2, hashMerkleRoot2;
        maxhash.SetHex("ec4ae2e8fcae1d107f6bc64f8614c7fde5d66b1a3edf770d63ca108078d83658");
        maxhash2.SetHex("cb125449651e708e2f4e004e1d49a0b837f2499cce0487aeec8dd77afa44eda7");
        hashMerkleRoot2.SetHex("d9f2a49b88a6a667ec31635b1148378d656eab79ba1bd4736cfe51516464980f");
        genesis = CreateGenesisBlock(1660121908, 3, 0x207fffff, 1, 50 * COIN, 1660121908, 2, 1660121908,  maxhash, maxhash2, hashMerkleRoot2);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("52e756ca5b01bcd06b456315b6a93402bb5f82afab6061428304d9eb3ec709be"));
        assert(genesis.hashMerkleRoot == uint256S("d9f2a49b88a6a667ec31635b1148378d656eab79ba1bd4736cfe51516464980f"));
	

        // Note that of those with the service bits flag, most only support a subset of possible options
	    //vSeeds.emplace_back("seed.bitcoin.sipa.be", true); // Pieter Wuille, only supports x1, x5, x9, and xd
	    //vSeeds.emplace_back("dnsseed.bluematt.me", true); // Matt Corallo, only supports x9
        //vSeeds.emplace_back("dnsseed.bitcoin.dashjr.org", false); // Luke Dashjr
        //vSeeds.emplace_back("seed.bitcoinstats.com", true); // Christian Decker, supports x1 - xf
        //vSeeds.emplace_back("seed.bitcoin.jonasschnelli.ch", true); // Jonas Schnelli, only supports x1, x5, x9, and xd
        //vSeeds.emplace_back("seed.btc.petertodd.org", true); // Peter Todd, only supports x1, x5, x9, and xd

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        //vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData) {
        };

        chainTxData = ChainTxData{
        };
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 21111;
        consensus.BIP34Hash = uint256S("0x0000000023b3a96d3484e5abb3755c413e7d41500f8e2a5c3f0dd01299cd8ef8");
        consensus.BIP65Height = 581885; // 00000000007f6655f22f98e72ed80d8b06dc761d5da09df0fa1dc4be4f861eb6
        consensus.BIP66Height = 330776; // 000000002104c8c45e99a8853285a3b592602a3ccde2b832481da85e9e4ba182
        consensus.powLimit = uint256S("0000007fffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1462060800; // May 1st 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1493596800; // May 1st 2017

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000002830dab7f76dbb7d63");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000002e9e7b00e1f6dc5123a04aad68dd0f0968d8c7aa45f6640795c37b1"); //1135275

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x07;
        nDefaultPort = 18333;
        nPruneAfterHeight = 1000;

#ifdef MINE_TESTNET_GENESIS
	uint32_t t = time(NULL);
	uint256 zero;
	zero.SetHex("0");
        fprintf(stderr, "Mining testnet genesis block...\n\ntime = %u\n", t);
        for (; ; ++t) {
            for (uint32_t n = 0; ; ++n) {
                if ((n & 0xfffff) == 0) fprintf(stderr, "\rnonce = %u", n);
		    uint32_t tn = time(NULL);
		    if(n == 0){
                	genesis = CreateGenesisBlock(t, n, 0x1d7fffff, 1, 50 * COIN, tn, n, tn, zero, zero, zero);
			continue;
		    }
		    else{
			if(UintToArith256(genesis.GetHash()) >= UintToArith256(genesis.maxhash)){
			    genesis = CreateGenesisBlock(t, n, 0x1d7fffff, 1, 50 * COIN, tn, genesis.nNonce, genesis.nTimeNonce, genesis.GetHash(), genesis.maxhash, genesis.hashMerkleRoot);
			}
			else{
			    genesis = CreateGenesisBlock(t, n, 0x1d7fffff, 1, 50 * COIN, tn, genesis.nNonce2, genesis.nTimeNonce2, genesis.maxhash, genesis.maxhash2, genesis.hashMerkleRoot2);
			}
		    }
                if (CheckProofOfWork(genesis.GetHash(), genesis.nBits, consensus)) {
                    fprintf(stderr,
                            "\rnonce = %u, %u\nnTime= %u\nnTimeNonce= %u\nmaxhash = %s,%s\nnNonce2 = %u\nnTimeNonce2 = %u\nmaxhash2 = %s\nhashMerkleRoot2 = %s\nhashMerkleRoot = %s\nhash = %s\n\n",
				n, genesis.nNonce, genesis.nTime, genesis.nTimeNonce, genesis.maxhash.ToString().c_str(), genesis.GetHash2().ToString().c_str(), genesis.nNonce2, genesis.nTimeNonce2,
				genesis.maxhash2.ToString().c_str(),genesis.hashMerkleRoot2.ToString().c_str(),genesis.hashMerkleRoot.ToString().c_str(),genesis.GetHash().ToString().c_str());
                    assert(false);
                }
                if (n == 4294967295) break;
            }
        }
#endif
	uint256 maxhash, maxhash2, hashMerkleRoot2;
	maxhash.SetHex("fffffb6bae138c879aeea4e5f495819a0ab1d6483442a615cf3ada13ad995cdb");
	maxhash2.SetHex("fffff82b3d81fb15904879a9c354aa0a7ae621b67e2414625472b5f01715cd39");
	hashMerkleRoot2.SetHex("4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b");
	genesis = CreateGenesisBlock(1517468520, 5696118, 0x1d7fffff, 1, 50 * COIN, 1517468567, 1984589, 1517468537, maxhash, maxhash2, hashMerkleRoot2);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0000001280c1287a51453d05235308f0892e9a0a7eac015385a9f746ed4f704b"));
        assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));


        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.emplace_back("testnet-seed.bitcoin.jonasschnelli.ch", true);
        vSeeds.emplace_back("seed.tbtc.petertodd.org", true);
        vSeeds.emplace_back("testnet-seed.bluematt.me", false);
        vSeeds.emplace_back("testnet-seed.bitcoin.schildbach.de", false);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        checkpointData = (CCheckpointData) {
            {
                {546, uint256S("000000002a936ca763904c3c35fce2f3556c559c0214345d31b1bcebf76acb70")},
            }
        };

        chainTxData = ChainTxData{
            // Data as of block 00000000000001c200b9790dc637d3bb141fe77d155b966ed775b17e109f7c6c (height 1156179)
            1501802953,
            14706531,
            0.15
        };

    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;
//b04902091
#ifdef MINE_REGTEST_GENESIS
        uint32_t t = time(NULL);
	uint256 zero;
	zero.SetHex("0");
        fprintf(stderr, "Mining regtest genesis block...\n\ntime = %u\n", t);
        for (; ; ++t) {
            for (uint32_t n = 0; ; ++n) {
                if ((n & 0xfffff) == 0) fprintf(stderr, "\rnonce = %u", n);
		    uint32_t tn = time(NULL);
		    if(n == 0){
                	genesis = CreateGenesisBlock(t, n, 0x207fffff, 1, 50 * COIN, tn, n, tn, zero, zero, zero);
			continue;
		    }
		    else{
			if(UintToArith256(genesis.GetHash()) >= UintToArith256(genesis.maxhash)){
			    genesis = CreateGenesisBlock(t, n, 0x207fffff, 1, 50 * COIN, tn, genesis.nNonce, genesis.nTimeNonce, genesis.GetHash(), genesis.maxhash, genesis.hashMerkleRoot);
			}
			else{
			    genesis = CreateGenesisBlock(t, n, 0x207fffff, 1, 50 * COIN, tn, genesis.nNonce2, genesis.nTimeNonce2, genesis.maxhash, genesis.maxhash2, genesis.hashMerkleRoot2);
			}
		    }
                if (CheckProofOfWork(genesis.GetHash(), genesis.nBits, consensus)) {
                    fprintf(stderr,
                            "\rnonce = %u, %u\nnTime= %u\nnTimeNonce= %u\nmaxhash = %s,%s\nnNonce2 = %u\nnTimeNonce2 = %u\nmaxhash2 = %s\nhashMerkleRoot2 = %s\nhashMerkleRoot = %s\nhash = %s\n\n",
				n, genesis.nNonce, genesis.nTime, genesis.nTimeNonce, genesis.maxhash.ToString().c_str(), genesis.GetHash2().ToString().c_str(), genesis.nNonce2, genesis.nTimeNonce2,
				genesis.maxhash2.ToString().c_str(),genesis.hashMerkleRoot2.ToString().c_str(),genesis.hashMerkleRoot.ToString().c_str(),genesis.GetHash().ToString().c_str());

                    assert(false);
                }
                if (n == 4294967295) break;
            }
        }
#endif
	uint256 maxhash, maxhash2, hashMerkleRoot2;
	maxhash.SetHex("ec4ae2e8fcae1d107f6bc64f8614c7fde5d66b1a3edf770d63ca108078d83658");
	maxhash2.SetHex("cb125449651e708e2f4e004e1d49a0b837f2499cce0487aeec8dd77afa44eda7");
	hashMerkleRoot2.SetHex("d9f2a49b88a6a667ec31635b1148378d656eab79ba1bd4736cfe51516464980f");
	genesis = CreateGenesisBlock(1660121908, 3, 0x207fffff, 1, 50 * COIN, 1660121908, 2, 1660121908,  maxhash, maxhash2, hashMerkleRoot2);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("52e756ca5b01bcd06b456315b6a93402bb5f82afab6061428304d9eb3ec709be"));
        assert(genesis.hashMerkleRoot == uint256S("d9f2a49b88a6a667ec31635b1148378d656eab79ba1bd4736cfe51516464980f"));


//b04902091
        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = (CCheckpointData) {
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
