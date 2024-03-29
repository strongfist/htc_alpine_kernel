



#include "precomp.h"

#if CFG_RSN_MIGRATION









BOOLEAN rsnParseRsnIE(IN P_ADAPTER_T prAdapter, IN P_RSN_INFO_ELEM_T prInfoElem, OUT P_RSN_INFO_T prRsnInfo)
{
	UINT_32 i;
	INT_32 u4RemainRsnIeLen;
	UINT_16 u2Version;
	UINT_16 u2Cap = 0;
	UINT_32 u4GroupSuite = RSN_CIPHER_SUITE_CCMP;
	UINT_16 u2PairSuiteCount = 0;
	UINT_16 u2AuthSuiteCount = 0;
	PUINT_8 pucPairSuite = NULL;
	PUINT_8 pucAuthSuite = NULL;
	PUINT_8 cp;

	DEBUGFUNC("rsnParseRsnIE");

	ASSERT(prInfoElem);
	ASSERT(prRsnInfo);

	
	if (prInfoElem->ucLength < 2) {
		DBGLOG(RSN, TRACE, "RSN IE length too short (length=%d)\n", prInfoElem->ucLength);
		return FALSE;
	}

	
	WLAN_GET_FIELD_16(&prInfoElem->u2Version, &u2Version);
	if (u2Version != 1) {
		DBGLOG(RSN, TRACE, "Unsupported RSN IE version: %d\n", u2Version);
		return FALSE;
	}

	cp = (PUCHAR) & prInfoElem->u4GroupKeyCipherSuite;
	u4RemainRsnIeLen = (INT_32) prInfoElem->ucLength - 2;

	do {
		if (u4RemainRsnIeLen == 0)
			break;

		
		if (u4RemainRsnIeLen < 4) {
			DBGLOG(RSN, TRACE, "Fail to parse RSN IE in group cipher suite (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		WLAN_GET_FIELD_32(cp, &u4GroupSuite);
		cp += 4;
		u4RemainRsnIeLen -= 4;

		if (u4RemainRsnIeLen == 0)
			break;

		
		if (u4RemainRsnIeLen < 2) {
			DBGLOG(RSN, TRACE, "Fail to parse RSN IE in pairwise cipher suite count (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		WLAN_GET_FIELD_16(cp, &u2PairSuiteCount);
		cp += 2;
		u4RemainRsnIeLen -= 2;

		
		i = (UINT_32) u2PairSuiteCount * 4;
		if (u4RemainRsnIeLen < (INT_32) i) {
			DBGLOG(RSN, TRACE, "Fail to parse RSN IE in pairwise cipher suite list (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		pucPairSuite = cp;

		cp += i;
		u4RemainRsnIeLen -= (INT_32) i;

		if (u4RemainRsnIeLen == 0)
			break;

		
		if (u4RemainRsnIeLen < 2) {
			DBGLOG(RSN, TRACE, "Fail to parse RSN IE in auth & key mgt suite count (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		WLAN_GET_FIELD_16(cp, &u2AuthSuiteCount);
		cp += 2;
		u4RemainRsnIeLen -= 2;

		i = (UINT_32) u2AuthSuiteCount * 4;
		if (u4RemainRsnIeLen < (INT_32) i) {
			DBGLOG(RSN, TRACE, "Fail to parse RSN IE in auth & key mgt suite list (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		pucAuthSuite = cp;

		cp += i;
		u4RemainRsnIeLen -= (INT_32) i;

		if (u4RemainRsnIeLen == 0)
			break;

		
		if (u4RemainRsnIeLen < 2) {
			DBGLOG(RSN, TRACE, "Fail to parse RSN IE in RSN capabilities (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		WLAN_GET_FIELD_16(cp, &u2Cap);
	} while (FALSE);

	
	prRsnInfo->ucElemId = ELEM_ID_RSN;

	prRsnInfo->u2Version = u2Version;

	prRsnInfo->u4GroupKeyCipherSuite = u4GroupSuite;

	DBGLOG(RSN, LOUD, "RSN: version %d, group key cipher suite %02x-%02x-%02x-%02x\n",
			   u2Version, (UCHAR) (u4GroupSuite & 0x000000FF),
			   (UCHAR) ((u4GroupSuite >> 8) & 0x000000FF),
			   (UCHAR) ((u4GroupSuite >> 16) & 0x000000FF), (UCHAR) ((u4GroupSuite >> 24) & 0x000000FF));

	if (pucPairSuite) {
		
		if (u2PairSuiteCount > MAX_NUM_SUPPORTED_CIPHER_SUITES)
			u2PairSuiteCount = MAX_NUM_SUPPORTED_CIPHER_SUITES;

		prRsnInfo->u4PairwiseKeyCipherSuiteCount = (UINT_32) u2PairSuiteCount;

		for (i = 0; i < (UINT_32) u2PairSuiteCount; i++) {
			WLAN_GET_FIELD_32(pucPairSuite, &prRsnInfo->au4PairwiseKeyCipherSuite[i]);
			pucPairSuite += 4;

			DBGLOG(RSN, LOUD, "RSN: pairwise key cipher suite [%d]: %02x-%02x-%02x-%02x\n",
					   (UINT_8) i, (UCHAR) (prRsnInfo->au4PairwiseKeyCipherSuite[i] & 0x000000FF),
					   (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[i] >> 8) & 0x000000FF),
					   (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[i] >> 16) & 0x000000FF),
					   (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[i] >> 24) & 0x000000FF));
		}
	} else {
		prRsnInfo->u4PairwiseKeyCipherSuiteCount = 1;
		prRsnInfo->au4PairwiseKeyCipherSuite[0] = RSN_CIPHER_SUITE_CCMP;

		DBGLOG(RSN, LOUD, "RSN: pairwise key cipher suite: %02x-%02x-%02x-%02x (default)\n",
				   (UCHAR) (prRsnInfo->au4PairwiseKeyCipherSuite[0] & 0x000000FF),
				   (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[0] >> 8) & 0x000000FF),
				   (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[0] >> 16) & 0x000000FF),
				   (UCHAR) ((prRsnInfo->au4PairwiseKeyCipherSuite[0] >> 24) & 0x000000FF));
	}

	if (pucAuthSuite) {
		if (u2AuthSuiteCount > MAX_NUM_SUPPORTED_AKM_SUITES)
			u2AuthSuiteCount = MAX_NUM_SUPPORTED_AKM_SUITES;

		prRsnInfo->u4AuthKeyMgtSuiteCount = (UINT_32) u2AuthSuiteCount;

		for (i = 0; i < (UINT_32) u2AuthSuiteCount; i++) {
			WLAN_GET_FIELD_32(pucAuthSuite, &prRsnInfo->au4AuthKeyMgtSuite[i]);
			pucAuthSuite += 4;

			DBGLOG(RSN, LOUD, "RSN: AKM suite [%d]: %02x-%02x-%02x-%02x\n",
					   (UINT_8) i, (UCHAR) (prRsnInfo->au4AuthKeyMgtSuite[i] & 0x000000FF),
					   (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[i] >> 8) & 0x000000FF),
					   (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[i] >> 16) & 0x000000FF),
					   (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[i] >> 24) & 0x000000FF));
		}
	} else {
		prRsnInfo->u4AuthKeyMgtSuiteCount = 1;
		prRsnInfo->au4AuthKeyMgtSuite[0] = RSN_AKM_SUITE_802_1X;

		DBGLOG(RSN, LOUD, "RSN: AKM suite: %02x-%02x-%02x-%02x (default)\n",
				   (UCHAR) (prRsnInfo->au4AuthKeyMgtSuite[0] & 0x000000FF),
				   (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[0] >> 8) & 0x000000FF),
				   (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[0] >> 16) & 0x000000FF),
				   (UCHAR) ((prRsnInfo->au4AuthKeyMgtSuite[0] >> 24) & 0x000000FF));
	}

	prRsnInfo->u2RsnCap = u2Cap;
#if CFG_SUPPORT_802_11W
	prRsnInfo->fgRsnCapPresent = TRUE;
#endif
	DBGLOG(RSN, LOUD, "RSN cap: 0x%04x\n", prRsnInfo->u2RsnCap);

	return TRUE;
}				

BOOLEAN rsnParseWpaIE(IN P_ADAPTER_T prAdapter, IN P_WPA_INFO_ELEM_T prInfoElem, OUT P_RSN_INFO_T prWpaInfo)
{
	UINT_32 i;
	INT_32 u4RemainWpaIeLen;
	UINT_16 u2Version;
	UINT_16 u2Cap = 0;
	UINT_32 u4GroupSuite = WPA_CIPHER_SUITE_TKIP;
	UINT_16 u2PairSuiteCount = 0;
	UINT_16 u2AuthSuiteCount = 0;
	PUCHAR pucPairSuite = NULL;
	PUCHAR pucAuthSuite = NULL;
	PUCHAR cp;
	BOOLEAN fgCapPresent = FALSE;

	DEBUGFUNC("rsnParseWpaIE");

	ASSERT(prInfoElem);
	ASSERT(prWpaInfo);

	
	if (prInfoElem->ucLength < 6) {
		DBGLOG(RSN, TRACE, "WPA IE length too short (length=%d)\n", prInfoElem->ucLength);
		return FALSE;
	}

	
	WLAN_GET_FIELD_16(&prInfoElem->u2Version, &u2Version);
	if (u2Version != 1) {
		DBGLOG(RSN, TRACE, "Unsupported WPA IE version: %d\n", u2Version);
		return FALSE;
	}

	cp = (PUCHAR) &prInfoElem->u4GroupKeyCipherSuite;
	u4RemainWpaIeLen = (INT_32) prInfoElem->ucLength - 6;

	do {
		if (u4RemainWpaIeLen == 0)
			break;


		
		if (u4RemainWpaIeLen < 4) {
			DBGLOG(RSN, TRACE, "Fail to parse WPA IE in group cipher suite (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		WLAN_GET_FIELD_32(cp, &u4GroupSuite);
		cp += 4;
		u4RemainWpaIeLen -= 4;

		if (u4RemainWpaIeLen == 0)
			break;

		
		if (u4RemainWpaIeLen < 2) {
			DBGLOG(RSN, TRACE, "Fail to parse WPA IE in pairwise cipher suite count (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		WLAN_GET_FIELD_16(cp, &u2PairSuiteCount);
		cp += 2;
		u4RemainWpaIeLen -= 2;

		
		i = (UINT_32) u2PairSuiteCount * 4;
		if (u4RemainWpaIeLen < (INT_32) i) {
			DBGLOG(RSN, TRACE, "Fail to parse WPA IE in pairwise cipher suite list (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		pucPairSuite = cp;

		cp += i;
		u4RemainWpaIeLen -= (INT_32) i;

		if (u4RemainWpaIeLen == 0)
			break;

		if (u4RemainWpaIeLen < 2) {
			DBGLOG(RSN, TRACE, "Fail to parse WPA IE in auth & key mgt suite count (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		WLAN_GET_FIELD_16(cp, &u2AuthSuiteCount);
		cp += 2;
		u4RemainWpaIeLen -= 2;

		i = (UINT_32) u2AuthSuiteCount * 4;
		if (u4RemainWpaIeLen < (INT_32) i) {
			DBGLOG(RSN, TRACE, "Fail to parse WPA IE in auth & key mgt suite list (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		pucAuthSuite = cp;

		cp += i;
		u4RemainWpaIeLen -= (INT_32) i;

		if (u4RemainWpaIeLen == 0)
			break;

		
		if (u4RemainWpaIeLen < 2) {
			DBGLOG(RSN, TRACE, "Fail to parse WPA IE in WPA capabilities (IE len: %d)\n",
					    prInfoElem->ucLength);
			return FALSE;
		}

		fgCapPresent = TRUE;
		WLAN_GET_FIELD_16(cp, &u2Cap);
		u4RemainWpaIeLen -= 2;
	} while (FALSE);

	

	prWpaInfo->ucElemId = ELEM_ID_WPA;

	prWpaInfo->u2Version = u2Version;

	prWpaInfo->u4GroupKeyCipherSuite = u4GroupSuite;

	DBGLOG(RSN, LOUD, "WPA: version %d, group key cipher suite %02x-%02x-%02x-%02x\n",
			   u2Version, (UCHAR) (u4GroupSuite & 0x000000FF),
			   (UCHAR) ((u4GroupSuite >> 8) & 0x000000FF),
			   (UCHAR) ((u4GroupSuite >> 16) & 0x000000FF), (UCHAR) ((u4GroupSuite >> 24) & 0x000000FF));

	if (pucPairSuite) {
		
		if (u2PairSuiteCount > MAX_NUM_SUPPORTED_CIPHER_SUITES)
			u2PairSuiteCount = MAX_NUM_SUPPORTED_CIPHER_SUITES;

		prWpaInfo->u4PairwiseKeyCipherSuiteCount = (UINT_32) u2PairSuiteCount;

		for (i = 0; i < (UINT_32) u2PairSuiteCount; i++) {
			WLAN_GET_FIELD_32(pucPairSuite, &prWpaInfo->au4PairwiseKeyCipherSuite[i]);
			pucPairSuite += 4;

			DBGLOG(RSN, LOUD, "WPA: pairwise key cipher suite [%d]: %02x-%02x-%02x-%02x\n",
					   (UINT_8) i, (UCHAR) (prWpaInfo->au4PairwiseKeyCipherSuite[i] & 0x000000FF),
					   (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[i] >> 8) & 0x000000FF),
					   (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[i] >> 16) & 0x000000FF),
					   (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[i] >> 24) & 0x000000FF));
		}
	} else {
		prWpaInfo->u4PairwiseKeyCipherSuiteCount = 1;
		prWpaInfo->au4PairwiseKeyCipherSuite[0] = WPA_CIPHER_SUITE_TKIP;

		DBGLOG(RSN, LOUD, "WPA: pairwise key cipher suite: %02x-%02x-%02x-%02x (default)\n",
				   (UCHAR) (prWpaInfo->au4PairwiseKeyCipherSuite[0] & 0x000000FF),
				   (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[0] >> 8) & 0x000000FF),
				   (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[0] >> 16) & 0x000000FF),
				   (UCHAR) ((prWpaInfo->au4PairwiseKeyCipherSuite[0] >> 24) & 0x000000FF));
	}

	if (pucAuthSuite) {
		if (u2AuthSuiteCount > MAX_NUM_SUPPORTED_AKM_SUITES)
			u2AuthSuiteCount = MAX_NUM_SUPPORTED_AKM_SUITES;

		prWpaInfo->u4AuthKeyMgtSuiteCount = (UINT_32) u2AuthSuiteCount;

		for (i = 0; i < (UINT_32) u2AuthSuiteCount; i++) {
			WLAN_GET_FIELD_32(pucAuthSuite, &prWpaInfo->au4AuthKeyMgtSuite[i]);
			pucAuthSuite += 4;

			DBGLOG(RSN, LOUD, "WPA: AKM suite [%d]: %02x-%02x-%02x-%02x\n",
					   (UINT_8) i, (UCHAR) (prWpaInfo->au4AuthKeyMgtSuite[i] & 0x000000FF),
					   (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[i] >> 8) & 0x000000FF),
					   (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[i] >> 16) & 0x000000FF),
					   (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[i] >> 24) & 0x000000FF));
		}
	} else {
		prWpaInfo->u4AuthKeyMgtSuiteCount = 1;
		prWpaInfo->au4AuthKeyMgtSuite[0] = WPA_AKM_SUITE_802_1X;

		DBGLOG(RSN, LOUD, "WPA: AKM suite: %02x-%02x-%02x-%02x (default)\n",
				   (UCHAR) (prWpaInfo->au4AuthKeyMgtSuite[0] & 0x000000FF),
				   (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[0] >> 8) & 0x000000FF),
				   (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[0] >> 16) & 0x000000FF),
				   (UCHAR) ((prWpaInfo->au4AuthKeyMgtSuite[0] >> 24) & 0x000000FF));
	}

	if (fgCapPresent) {
		prWpaInfo->fgRsnCapPresent = TRUE;
		prWpaInfo->u2RsnCap = u2Cap;
		DBGLOG(RSN, LOUD, "WPA: RSN cap: 0x%04x\n", prWpaInfo->u2RsnCap);
	} else {
		prWpaInfo->fgRsnCapPresent = FALSE;
		prWpaInfo->u2RsnCap = 0;
	}

	return TRUE;
}				

BOOLEAN rsnSearchSupportedCipher(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Cipher, OUT PUINT_32 pu4Index)
{
	UINT_8 i;
	P_DOT11_RSNA_CONFIG_PAIRWISE_CIPHERS_ENTRY prEntry;

	DEBUGFUNC("rsnSearchSupportedCipher");

	ASSERT(pu4Index);

	for (i = 0; i < MAX_NUM_SUPPORTED_CIPHER_SUITES; i++) {
		prEntry = &prAdapter->rMib.dot11RSNAConfigPairwiseCiphersTable[i];
		if (prEntry->dot11RSNAConfigPairwiseCipher == u4Cipher &&
			prEntry->dot11RSNAConfigPairwiseCipherEnabled) {
			*pu4Index = i;
			return TRUE;
		}
	}
	return FALSE;
}				

BOOLEAN rsnIsSuitableBSS(IN P_ADAPTER_T prAdapter, IN P_RSN_INFO_T prBssRsnInfo)
{
	UINT_8 i = 0;

	DEBUGFUNC("rsnIsSuitableBSS");

	do {

		if ((prAdapter->rWifiVar.rConnSettings.rRsnInfo.u4GroupKeyCipherSuite & 0x000000FF) !=
		    GET_SELECTOR_TYPE(prBssRsnInfo->u4GroupKeyCipherSuite)) {
			DBGLOG(RSN, TRACE, "Break by GroupKeyCipherSuite\n");
			break;
		}
		for (i = 0; i < prBssRsnInfo->u4PairwiseKeyCipherSuiteCount; i++) {
			if (((prAdapter->rWifiVar.rConnSettings.rRsnInfo.au4PairwiseKeyCipherSuite[0] & 0x000000FF) !=
			     GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[i]))
			    && (i == prBssRsnInfo->u4PairwiseKeyCipherSuiteCount - 1)) {
				DBGLOG(RSN, TRACE, "Break by PairwiseKeyCipherSuite\n");
				break;
			}
		}
		for (i = 0; i < prBssRsnInfo->u4AuthKeyMgtSuiteCount; i++) {
			if (((prAdapter->rWifiVar.rConnSettings.rRsnInfo.au4AuthKeyMgtSuite[0] & 0x000000FF) !=
			     GET_SELECTOR_TYPE(prBssRsnInfo->au4AuthKeyMgtSuite[0]))
			    && (i == prBssRsnInfo->u4AuthKeyMgtSuiteCount - 1)) {
				DBGLOG(RSN, TRACE, "Break by AuthKeyMgtSuite\n");
				break;
			}
		}
		return TRUE;
	} while (FALSE);
	return FALSE;
}

BOOLEAN rsnSearchAKMSuite(IN P_ADAPTER_T prAdapter, IN UINT_32 u4AkmSuite, OUT PUINT_32 pu4Index)
{
	UINT_8 i;
	P_DOT11_RSNA_CONFIG_AUTHENTICATION_SUITES_ENTRY prEntry;

	DEBUGFUNC("rsnSearchAKMSuite");

	ASSERT(pu4Index);

	for (i = 0; i < MAX_NUM_SUPPORTED_AKM_SUITES; i++) {
		prEntry = &prAdapter->rMib.dot11RSNAConfigAuthenticationSuitesTable[i];
		if (prEntry->dot11RSNAConfigAuthenticationSuite == u4AkmSuite &&
		    prEntry->dot11RSNAConfigAuthenticationSuiteEnabled) {
			*pu4Index = i;
			return TRUE;
		}
	}
	return FALSE;
}				

BOOLEAN rsnPerformPolicySelection(IN P_ADAPTER_T prAdapter, IN P_BSS_DESC_T prBss)
{
#if CFG_SUPPORT_802_11W
	INT_32 i;
	UINT_32 j;
#else
	UINT_32 i, j;
#endif
	BOOLEAN fgSuiteSupported;
	UINT_32 u4PairwiseCipher = 0;
	UINT_32 u4GroupCipher = 0;
	UINT_32 u4AkmSuite = 0;
	P_RSN_INFO_T prBssRsnInfo;
	ENUM_NETWORK_TYPE_INDEX_T eNetwotkType;
	BOOLEAN fgIsWpsActive = (BOOLEAN) FALSE;

	DEBUGFUNC("rsnPerformPolicySelection");

	ASSERT(prBss);

	DBGLOG(RSN, TRACE, "rsnPerformPolicySelection\n");
	
	eNetwotkType = NETWORK_TYPE_AIS_INDEX;

	prBss->u4RsnSelectedPairwiseCipher = 0;
	prBss->u4RsnSelectedGroupCipher = 0;
	prBss->u4RsnSelectedAKMSuite = 0;
	prBss->ucEncLevel = 0;

#if CFG_SUPPORT_WPS
	fgIsWpsActive = kalWSCGetActiveState(prAdapter->prGlueInfo);

	
	if (fgIsWpsActive &&
	    (prAdapter->rWifiVar.rConnSettings.eAuthMode < AUTH_MODE_WPA) &&
	    (prAdapter->rWifiVar.rConnSettings.eOPMode == NET_TYPE_INFRA)) {
		DBGLOG(RSN, TRACE, "-- Skip the Protected BSS check\n");
		return TRUE;
	}
#endif

	
	if ((prBss->u2CapInfo & CAP_INFO_PRIVACY) == 0) {

		if (secEnabledInAis(prAdapter) == FALSE) {
			DBGLOG(RSN, TRACE, "-- No Protected BSS\n");
			return TRUE;
		}
		DBGLOG(RSN, TRACE, "-- Protected BSS\n");
		return FALSE;

	}

	
	if ((prBss->u2CapInfo & CAP_INFO_PRIVACY) != 0) {
		if (secEnabledInAis(prAdapter) == FALSE) {
			DBGLOG(RSN, TRACE, "-- Protected BSS\n");
			return FALSE;
		}
	}

	if (prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA ||
	    prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA_PSK ||
	    prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA_NONE) {

		if (prBss->fgIEWPA) {
			prBssRsnInfo = &prBss->rWPAInfo;
		} else {
			DBGLOG(RSN, TRACE, "WPA Information Element does not exist.\n");
			return FALSE;
		}
	} else if (prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA2 ||
		   prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA2_PSK) {

		if (prBss->fgIERSN) {
			prBssRsnInfo = &prBss->rRSNInfo;
		} else {
			DBGLOG(RSN, TRACE, "RSN Information Element does not exist.\n");
			return FALSE;
		}
	} else if (prAdapter->rWifiVar.rConnSettings.eEncStatus != ENUM_ENCRYPTION1_ENABLED) {
		
		DBGLOG(RSN, TRACE, "-- Not WEP-only legacy BSS %d\n", prAdapter->rWifiVar.rConnSettings.eEncStatus);
		return FALSE;
	} else if (prAdapter->rWifiVar.rConnSettings.eEncStatus == ENUM_ENCRYPTION1_ENABLED) {
		
		DBGLOG(RSN, TRACE, "-- WEP-only legacy BSS, fgIERSN %d, fgIEWPA %d\n",
				    prBss->fgIERSN, prBss->fgIEWPA);
		
		return (prBss->fgIERSN || prBss->fgIEWPA) ? FALSE : TRUE;
	}

	if (!rsnIsSuitableBSS(prAdapter, prBssRsnInfo)) {
		DBGLOG(RSN, TRACE, "RSN info check no matched\n");
		return FALSE;
	}

	if (prBssRsnInfo->u4PairwiseKeyCipherSuiteCount == 1 &&
	    GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[0]) == CIPHER_SUITE_NONE) {
		fgSuiteSupported = FALSE;

		switch (prBssRsnInfo->u4GroupKeyCipherSuite) {
		case WPA_CIPHER_SUITE_CCMP:
		case RSN_CIPHER_SUITE_CCMP:
			if (prAdapter->rWifiVar.rConnSettings.eEncStatus == ENUM_ENCRYPTION3_ENABLED)
				fgSuiteSupported = TRUE;
			break;

		case WPA_CIPHER_SUITE_TKIP:
		case RSN_CIPHER_SUITE_TKIP:
			if (prAdapter->rWifiVar.rConnSettings.eEncStatus == ENUM_ENCRYPTION2_ENABLED)
				fgSuiteSupported = TRUE;
			break;

		case WPA_CIPHER_SUITE_WEP40:
		case WPA_CIPHER_SUITE_WEP104:
			if (prAdapter->rWifiVar.rConnSettings.eEncStatus == ENUM_ENCRYPTION1_ENABLED)
				fgSuiteSupported = TRUE;
			break;
		}

		if (fgSuiteSupported) {
			u4PairwiseCipher = WPA_CIPHER_SUITE_NONE;
			u4GroupCipher = prBssRsnInfo->u4GroupKeyCipherSuite;
		}
#if DBG
		else {
			DBGLOG(RSN, TRACE, "Inproper encryption status %d for group-key-only BSS\n",
					    prAdapter->rWifiVar.rConnSettings.eEncStatus);
		}
#endif
	} else {
		fgSuiteSupported = FALSE;

		DBGLOG(RSN, TRACE, "eEncStatus %d %d 0x%x\n", prAdapter->rWifiVar.rConnSettings.eEncStatus,
				    (UINT_32) prBssRsnInfo->u4PairwiseKeyCipherSuiteCount,
				    (UINT_32) prBssRsnInfo->au4PairwiseKeyCipherSuite[0]);
		
		switch (prAdapter->rWifiVar.rConnSettings.eEncStatus) {
		case ENUM_ENCRYPTION3_ENABLED:
			for (i = 0; i < prBssRsnInfo->u4PairwiseKeyCipherSuiteCount; i++) {
				if (GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[i])
				    == CIPHER_SUITE_CCMP) {
					u4PairwiseCipher = prBssRsnInfo->au4PairwiseKeyCipherSuite[i];
				}
			}
			u4GroupCipher = prBssRsnInfo->u4GroupKeyCipherSuite;
			break;

		case ENUM_ENCRYPTION2_ENABLED:
			for (i = 0; i < prBssRsnInfo->u4PairwiseKeyCipherSuiteCount; i++) {
				if (GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[i])
				    == CIPHER_SUITE_TKIP) {
					u4PairwiseCipher = prBssRsnInfo->au4PairwiseKeyCipherSuite[i];
				}
			}
			if (GET_SELECTOR_TYPE(prBssRsnInfo->u4GroupKeyCipherSuite) == CIPHER_SUITE_CCMP)
				DBGLOG(RSN, TRACE, "Cannot join CCMP BSS\n");
			else
				u4GroupCipher = prBssRsnInfo->u4GroupKeyCipherSuite;
			break;

		case ENUM_ENCRYPTION1_ENABLED:
			for (i = 0; i < prBssRsnInfo->u4PairwiseKeyCipherSuiteCount; i++) {
				if (GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[i])
				    == CIPHER_SUITE_WEP40 ||
				    GET_SELECTOR_TYPE(prBssRsnInfo->au4PairwiseKeyCipherSuite[i])
				    == CIPHER_SUITE_WEP104) {
					u4PairwiseCipher = prBssRsnInfo->au4PairwiseKeyCipherSuite[i];
				}
			}
			if (GET_SELECTOR_TYPE(prBssRsnInfo->u4GroupKeyCipherSuite) ==
			    CIPHER_SUITE_CCMP ||
			    GET_SELECTOR_TYPE(prBssRsnInfo->u4GroupKeyCipherSuite) == CIPHER_SUITE_TKIP) {
				DBGLOG(RSN, TRACE, "Cannot join CCMP/TKIP BSS\n");
			} else {
				u4GroupCipher = prBssRsnInfo->u4GroupKeyCipherSuite;
			}
			break;

		default:
			break;
		}
	}

	
	if (u4PairwiseCipher == 0 || u4GroupCipher == 0) {
		DBGLOG(RSN, TRACE, "Failed to select pairwise/group cipher (0x%08x/0x%08x)\n",
				    u4PairwiseCipher, u4GroupCipher);
		return FALSE;
	}
#if CFG_ENABLE_WIFI_DIRECT
	if ((prAdapter->fgIsP2PRegistered) && (eNetwotkType == NETWORK_TYPE_P2P_INDEX)) {
		if (u4PairwiseCipher != RSN_CIPHER_SUITE_CCMP ||
		    u4GroupCipher != RSN_CIPHER_SUITE_CCMP || u4AkmSuite != RSN_AKM_SUITE_PSK) {
			DBGLOG(RSN, TRACE, "Failed to select pairwise/group cipher for P2P network (0x%08x/0x%08x)\n",
					    u4PairwiseCipher, u4GroupCipher);
			return FALSE;
		}
	}
#endif

#if CFG_ENABLE_BT_OVER_WIFI
	if (eNetwotkType == NETWORK_TYPE_BOW_INDEX) {
		if (u4PairwiseCipher != RSN_CIPHER_SUITE_CCMP ||
			u4GroupCipher != RSN_CIPHER_SUITE_CCMP || u4AkmSuite != RSN_AKM_SUITE_PSK) {
			
		}
		DBGLOG(RSN, TRACE,
		       "Failed to select pairwise/group cipher for BT over Wi-Fi network (0x%08x/0x%08x)\n",
			u4PairwiseCipher, u4GroupCipher);
		return FALSE;
	}
#endif

	
	fgSuiteSupported = rsnSearchSupportedCipher(prAdapter, u4PairwiseCipher, &i);

	
	if (fgSuiteSupported)
		fgSuiteSupported = rsnSearchSupportedCipher(prAdapter, u4GroupCipher, &i);

	if (!fgSuiteSupported) {
		DBGLOG(RSN, TRACE, "Failed to support selected pairwise/group cipher (0x%08x/0x%08x)\n",
				    u4PairwiseCipher, u4GroupCipher);
		return FALSE;
	}

	
	
#if CFG_SUPPORT_802_11W
	if (i != 0)
		for (i = (prBssRsnInfo->u4AuthKeyMgtSuiteCount - 1); i >= 0; i--) {
#else
	for (i = 0; i < prBssRsnInfo->u4AuthKeyMgtSuiteCount; i++) {
#endif
		if (rsnSearchAKMSuite(prAdapter, prBssRsnInfo->au4AuthKeyMgtSuite[i], &j)) {
			u4AkmSuite = prBssRsnInfo->au4AuthKeyMgtSuite[i];
			break;
		}
	}

	if (u4AkmSuite == 0) {
		DBGLOG(RSN, TRACE, "Cannot support any AKM suites\n");
		return FALSE;
	}

	DBGLOG(RSN, TRACE, "Selected pairwise/group cipher: %02x-%02x-%02x-%02x/%02x-%02x-%02x-%02x\n",
			    (UINT_8) (u4PairwiseCipher & 0x000000FF),
			    (UINT_8) ((u4PairwiseCipher >> 8) & 0x000000FF),
			    (UINT_8) ((u4PairwiseCipher >> 16) & 0x000000FF),
			    (UINT_8) ((u4PairwiseCipher >> 24) & 0x000000FF),
			    (UINT_8) (u4GroupCipher & 0x000000FF),
			    (UINT_8) ((u4GroupCipher >> 8) & 0x000000FF),
			    (UINT_8) ((u4GroupCipher >> 16) & 0x000000FF),
			    (UINT_8) ((u4GroupCipher >> 24) & 0x000000FF));

	DBGLOG(RSN, TRACE, "Selected AKM suite: %02x-%02x-%02x-%02x\n",
			    (UINT_8) (u4AkmSuite & 0x000000FF),
			    (UINT_8) ((u4AkmSuite >> 8) & 0x000000FF),
			    (UINT_8) ((u4AkmSuite >> 16) & 0x000000FF), (UINT_8) ((u4AkmSuite >> 24) & 0x000000FF));

#if CFG_SUPPORT_802_11W
	DBGLOG(RSN, TRACE, "MFP setting = %d\n ", kalGetMfpSetting(prAdapter->prGlueInfo));

	if (kalGetMfpSetting(prAdapter->prGlueInfo) == RSN_AUTH_MFP_REQUIRED) {
		if (!prBssRsnInfo->fgRsnCapPresent) {
			DBGLOG(RSN, TRACE, "Skip RSN IE, No MFP Required Capability.\n");
			return FALSE;
		} else if (!(prBssRsnInfo->u2RsnCap & ELEM_WPA_CAP_MFPC)) {
			DBGLOG(RSN, TRACE, "Skip RSN IE, No MFP Required\n");
			return FALSE;
		}
		prAdapter->rWifiVar.rAisSpecificBssInfo.fgMgmtProtection = TRUE;
	} else if (kalGetMfpSetting(prAdapter->prGlueInfo) == RSN_AUTH_MFP_OPTIONAL) {
		if (prBssRsnInfo->u2RsnCap && ((prBssRsnInfo->u2RsnCap & ELEM_WPA_CAP_MFPR) ||
					       (prBssRsnInfo->u2RsnCap & ELEM_WPA_CAP_MFPC))) {
			prAdapter->rWifiVar.rAisSpecificBssInfo.fgMgmtProtection = TRUE;
		} else {
			prAdapter->rWifiVar.rAisSpecificBssInfo.fgMgmtProtection = FALSE;
		}
	} else {
		if (prBssRsnInfo->fgRsnCapPresent && (prBssRsnInfo->u2RsnCap & ELEM_WPA_CAP_MFPR)) {
			DBGLOG(RSN, TRACE, "Skip RSN IE, No MFP Required Capability\n");
			return FALSE;
		}
		prAdapter->rWifiVar.rAisSpecificBssInfo.fgMgmtProtection = FALSE;
	}
	DBGLOG(RSN, TRACE, "fgMgmtProtection = %d\n ", prAdapter->rWifiVar.rAisSpecificBssInfo.fgMgmtProtection);
#endif

	if (GET_SELECTOR_TYPE(u4GroupCipher) == CIPHER_SUITE_CCMP) {
		prBss->ucEncLevel = 3;
	} else if (GET_SELECTOR_TYPE(u4GroupCipher) == CIPHER_SUITE_TKIP) {
		prBss->ucEncLevel = 2;
	} else if (GET_SELECTOR_TYPE(u4GroupCipher) == CIPHER_SUITE_WEP40 ||
		   GET_SELECTOR_TYPE(u4GroupCipher) == CIPHER_SUITE_WEP104) {
		prBss->ucEncLevel = 1;
	} else {
		ASSERT(FALSE);
	}
	prBss->u4RsnSelectedPairwiseCipher = u4PairwiseCipher;
	prBss->u4RsnSelectedGroupCipher = u4GroupCipher;
	prBss->u4RsnSelectedAKMSuite = u4AkmSuite;

	return TRUE;

}				

VOID rsnGenerateWpaNoneIE(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo)
{
	UINT_32 i;
	P_WPA_INFO_ELEM_T prWpaIE;
	UINT_32 u4Suite;
	UINT_16 u2SuiteCount;
	PUINT_8 cp, cp2;
	UINT_8 ucExpendedLen = 0;
	PUINT_8 pucBuffer;
	ENUM_NETWORK_TYPE_INDEX_T eNetworkId;

	DEBUGFUNC("rsnGenerateWpaNoneIE");

	ASSERT(prMsduInfo);

	if (prAdapter->rWifiVar.rConnSettings.eAuthMode != AUTH_MODE_WPA_NONE)
		return;

	eNetworkId = (ENUM_NETWORK_TYPE_INDEX_T) prMsduInfo->ucNetworkType;

	if (eNetworkId != NETWORK_TYPE_AIS_INDEX)
		return;

	pucBuffer = (PUINT_8) ((ULONG) prMsduInfo->prPacket + (UINT_32) prMsduInfo->u2FrameLength);

	ASSERT(pucBuffer);

	prWpaIE = (P_WPA_INFO_ELEM_T) (pucBuffer);

	
	
	prWpaIE->ucElemId = ELEM_ID_WPA;

	
	prWpaIE->aucOui[0] = 0x00;
	prWpaIE->aucOui[1] = 0x50;
	prWpaIE->aucOui[2] = 0xF2;
	prWpaIE->ucOuiType = VENDOR_OUI_TYPE_WPA;

	
	WLAN_SET_FIELD_16(&prWpaIE->u2Version, 1);	
	ucExpendedLen = 6;

	
	u2SuiteCount = 0;
	cp = (PUINT_8) &prWpaIE->aucPairwiseKeyCipherSuite1[0];

	if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_CCMP, &i))
		u4Suite = WPA_CIPHER_SUITE_CCMP;
	else if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_TKIP, &i))
		u4Suite = WPA_CIPHER_SUITE_TKIP;
	else if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_WEP104, &i))
		u4Suite = WPA_CIPHER_SUITE_WEP104;
	else if (rsnSearchSupportedCipher(prAdapter, WPA_CIPHER_SUITE_WEP40, &i))
		u4Suite = WPA_CIPHER_SUITE_WEP40;
	else
		u4Suite = WPA_CIPHER_SUITE_TKIP;

	WLAN_SET_FIELD_32(cp, u4Suite);
	u2SuiteCount++;
	ucExpendedLen += 4;
	cp += 4;

	
	WLAN_SET_FIELD_32(&prWpaIE->u4GroupKeyCipherSuite, u4Suite);
	ucExpendedLen += 4;

	
	WLAN_SET_FIELD_16(&prWpaIE->u2PairwiseKeyCipherSuiteCount, u2SuiteCount);
	ucExpendedLen += 2;

	cp2 = cp;

	
	u2SuiteCount = 0;
	cp += 2;

	if (rsnSearchAKMSuite(prAdapter, WPA_AKM_SUITE_802_1X, &i))
		u4Suite = WPA_AKM_SUITE_802_1X;
	else if (rsnSearchAKMSuite(prAdapter, WPA_AKM_SUITE_PSK, &i))
		u4Suite = WPA_AKM_SUITE_PSK;
	else
		u4Suite = WPA_AKM_SUITE_NONE;

	
	ASSERT(u4Suite == WPA_AKM_SUITE_NONE);

	WLAN_SET_FIELD_32(cp, u4Suite);
	u2SuiteCount++;
	ucExpendedLen += 4;
	cp += 4;

	
	WLAN_SET_FIELD_16(cp2, u2SuiteCount);
	ucExpendedLen += 2;

	
	prWpaIE->ucLength = (UINT_8) ucExpendedLen;

	
	prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);

}				

VOID rsnGenerateWPAIE(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo)
{
	PUCHAR cp;
	PUINT_8 pucBuffer;
	ENUM_NETWORK_TYPE_INDEX_T eNetworkId;
	P_P2P_SPECIFIC_BSS_INFO_T prP2pSpecificBssInfo;

	DEBUGFUNC("rsnGenerateWPAIE");

	ASSERT(prMsduInfo);

	pucBuffer = (PUINT_8) ((ULONG) prMsduInfo->prPacket + (UINT_32) prMsduInfo->u2FrameLength);

	ASSERT(pucBuffer);

	eNetworkId = (ENUM_NETWORK_TYPE_INDEX_T) prMsduInfo->ucNetworkType;
	prP2pSpecificBssInfo = prAdapter->rWifiVar.prP2pSpecificBssInfo;

	
	

#if CFG_ENABLE_WIFI_DIRECT
	if ((1   &&
		((prAdapter->fgIsP2PRegistered) &&
		(eNetworkId == NETWORK_TYPE_P2P_INDEX) &&
		(kalP2PGetTkipCipher(prAdapter->prGlueInfo)))) ||
		((prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA) ||
		(prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA_PSK))) {
#else
	if ((1  &&
		((prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA) ||
		(prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA_PSK)))) {
#endif
		if (prAdapter->fgIsP2PRegistered && prP2pSpecificBssInfo && prP2pSpecificBssInfo->u2WpaIeLen != 0) {
			kalMemCopy(pucBuffer, prP2pSpecificBssInfo->aucWpaIeBuffer, prP2pSpecificBssInfo->u2WpaIeLen);
			prMsduInfo->u2FrameLength += prP2pSpecificBssInfo->u2WpaIeLen;
			return;
		}

		
		WPA_IE(pucBuffer)->ucElemId = ELEM_ID_WPA;
		WPA_IE(pucBuffer)->ucLength = ELEM_ID_WPA_LEN_FIXED;
		WPA_IE(pucBuffer)->aucOui[0] = 0x00;
		WPA_IE(pucBuffer)->aucOui[1] = 0x50;
		WPA_IE(pucBuffer)->aucOui[2] = 0xF2;
		WPA_IE(pucBuffer)->ucOuiType = VENDOR_OUI_TYPE_WPA;
		WLAN_SET_FIELD_16(&WPA_IE(pucBuffer)->u2Version, 1);

#if CFG_ENABLE_WIFI_DIRECT
		if (prAdapter->fgIsP2PRegistered && eNetworkId == NETWORK_TYPE_P2P_INDEX) {
			WLAN_SET_FIELD_32(&WPA_IE(pucBuffer)->u4GroupKeyCipherSuite, WPA_CIPHER_SUITE_TKIP);
		} else
#endif
			WLAN_SET_FIELD_32(&WPA_IE(pucBuffer)->u4GroupKeyCipherSuite,
					  prAdapter->rWifiVar.
					  arBssInfo[NETWORK_TYPE_AIS_INDEX].u4RsnSelectedGroupCipher);

		cp = (PUCHAR) &WPA_IE(pucBuffer)->aucPairwiseKeyCipherSuite1[0];

		WLAN_SET_FIELD_16(&WPA_IE(pucBuffer)->u2PairwiseKeyCipherSuiteCount, 1);
#if CFG_ENABLE_WIFI_DIRECT
		if (prAdapter->fgIsP2PRegistered && eNetworkId == NETWORK_TYPE_P2P_INDEX) {
			WLAN_SET_FIELD_32(cp, WPA_CIPHER_SUITE_TKIP);
		} else
#endif
			WLAN_SET_FIELD_32(cp,
					  prAdapter->rWifiVar.
					  arBssInfo[NETWORK_TYPE_AIS_INDEX].u4RsnSelectedPairwiseCipher);
		cp += 4;

		WLAN_SET_FIELD_16(cp, 1);
		cp += 2;
#if CFG_ENABLE_WIFI_DIRECT
		if (prAdapter->fgIsP2PRegistered && eNetworkId == NETWORK_TYPE_P2P_INDEX) {
			WLAN_SET_FIELD_32(cp, WPA_AKM_SUITE_PSK);
		} else
#endif
			WLAN_SET_FIELD_32(cp,
					  prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX].u4RsnSelectedAKMSuite);
		cp += 4;

		WPA_IE(pucBuffer)->ucLength = ELEM_ID_WPA_LEN_FIXED;

		prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);
	}

}				

VOID rsnGenerateRSNIE(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo)
{
	UINT_32 u4Entry;
	PUCHAR cp;
	
	PUINT_8 pucBuffer;
	ENUM_NETWORK_TYPE_INDEX_T eNetworkId;
	P_STA_RECORD_T prStaRec;

	DEBUGFUNC("rsnGenerateRSNIE");

	ASSERT(prMsduInfo);

	pucBuffer = (PUINT_8) ((ULONG) prMsduInfo->prPacket + (UINT_32) prMsduInfo->u2FrameLength);

	ASSERT(pucBuffer);

	
	eNetworkId = (ENUM_NETWORK_TYPE_INDEX_T) prMsduInfo->ucNetworkType;

	if (
#if CFG_ENABLE_WIFI_DIRECT
		   ((prAdapter->fgIsP2PRegistered) &&
		    (eNetworkId == NETWORK_TYPE_P2P_INDEX) && (kalP2PGetCcmpCipher(prAdapter->prGlueInfo))) ||
#endif
#if CFG_ENABLE_BT_OVER_WIFI
		   (eNetworkId == NETWORK_TYPE_BOW_INDEX) ||
#endif
		   (eNetworkId == NETWORK_TYPE_AIS_INDEX   &&
		    ((prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA2) ||
		     (prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA2_PSK)))) {
		
		RSN_IE(pucBuffer)->ucElemId = ELEM_ID_RSN;
		RSN_IE(pucBuffer)->ucLength = ELEM_ID_RSN_LEN_FIXED;
		WLAN_SET_FIELD_16(&RSN_IE(pucBuffer)->u2Version, 1);	
		WLAN_SET_FIELD_32(&RSN_IE(pucBuffer)->u4GroupKeyCipherSuite,
			prAdapter->rWifiVar.arBssInfo[eNetworkId].u4RsnSelectedGroupCipher);	
		cp = (PUCHAR) &RSN_IE(pucBuffer)->aucPairwiseKeyCipherSuite1[0];
		WLAN_SET_FIELD_16(&RSN_IE(pucBuffer)->u2PairwiseKeyCipherSuiteCount, 1);
		WLAN_SET_FIELD_32(cp, prAdapter->rWifiVar.arBssInfo[eNetworkId].u4RsnSelectedPairwiseCipher);
		cp += 4;
		WLAN_SET_FIELD_16(cp, 1);	
		cp += 2;
		WLAN_SET_FIELD_32(cp, prAdapter->rWifiVar.arBssInfo[eNetworkId].u4RsnSelectedAKMSuite);	
		cp += 4;
		WLAN_SET_FIELD_16(cp, prAdapter->rWifiVar.arBssInfo[eNetworkId].u2RsnSelectedCapInfo);
#if CFG_SUPPORT_802_11W
		if (eNetworkId == NETWORK_TYPE_AIS_INDEX && prAdapter->rWifiVar.rAisSpecificBssInfo.fgMgmtProtection) {
			if (kalGetMfpSetting(prAdapter->prGlueInfo) == RSN_AUTH_MFP_REQUIRED)
				WLAN_SET_FIELD_16(cp, ELEM_WPA_CAP_MFPC | ELEM_WPA_CAP_MFPR);	
			else if (kalGetMfpSetting(prAdapter->prGlueInfo) == RSN_AUTH_MFP_OPTIONAL)
				WLAN_SET_FIELD_16(cp, ELEM_WPA_CAP_MFPC);	
		}
#endif
		cp += 2;

		if (eNetworkId == NETWORK_TYPE_AIS_INDEX) {
			prStaRec = cnmGetStaRecByIndex(prAdapter, prMsduInfo->ucStaRecIndex);
			if (!prStaRec) {
				DBGLOG(RSN, TRACE, "rsnGenerateRSNIE: prStaRec is NULL\n");
				return;
			}
		}

		if (eNetworkId == NETWORK_TYPE_AIS_INDEX &&
		    rsnSearchPmkidEntry(prAdapter, prStaRec->aucMacAddr, &u4Entry)) {
			
			
			
		
			if (prAdapter->rWifiVar.rAisSpecificBssInfo.arPmkidCache[u4Entry].fgPmkidExist) {
				RSN_IE(pucBuffer)->ucLength = 38;
				WLAN_SET_FIELD_16(cp, 1);	
				cp += 2;
				DBGLOG(RSN, TRACE,
				       "BSSID %pM ind=%d\n", prStaRec->aucMacAddr, (UINT_32) u4Entry);
				DBGLOG(RSN, TRACE, "use PMKID %pM\n",
					(prAdapter->rWifiVar.rAisSpecificBssInfo.
						arPmkidCache[u4Entry].rBssidInfo.arPMKID));
				kalMemCopy(cp,
					   (PVOID) prAdapter->rWifiVar.rAisSpecificBssInfo.
					   arPmkidCache[u4Entry].rBssidInfo.arPMKID, sizeof(PARAM_PMKID_VALUE));
				
			} else {
				WLAN_SET_FIELD_16(cp, 0);	
				
#if CFG_SUPPORT_802_11W
				cp += 2;
				RSN_IE(pucBuffer)->ucLength += 2;
#endif
			}
		} else {
			WLAN_SET_FIELD_16(cp, 0);	
			
#if CFG_SUPPORT_802_11W
			cp += 2;
			RSN_IE(pucBuffer)->ucLength += 2;
#endif
		}

#if CFG_SUPPORT_802_11W
		if ((eNetworkId == NETWORK_TYPE_AIS_INDEX)
		    && (kalGetMfpSetting(prAdapter->prGlueInfo) !=
			RSN_AUTH_MFP_DISABLED) ) {
			WLAN_SET_FIELD_32(cp, RSN_CIPHER_SUITE_AES_128_CMAC);
			cp += 4;
			RSN_IE(pucBuffer)->ucLength += 4;
		}
#endif
		prMsduInfo->u2FrameLength += IE_SIZE(pucBuffer);
	}

}				

BOOLEAN
rsnParseCheckForWFAInfoElem(IN P_ADAPTER_T prAdapter,
			    IN PUINT_8 pucBuf, OUT PUINT_8 pucOuiType, OUT PUINT_16 pu2SubTypeVersion)
{
	UINT_8 aucWfaOui[] = VENDOR_OUI_WFA;
	P_IE_WFA_T prWfaIE;

	ASSERT(pucBuf);
	ASSERT(pucOuiType);
	ASSERT(pu2SubTypeVersion);
	prWfaIE = (P_IE_WFA_T) pucBuf;

	do {
		if (IE_LEN(pucBuf) <= ELEM_MIN_LEN_WFA_OUI_TYPE_SUBTYPE) {
			break;
		} else if (prWfaIE->aucOui[0] != aucWfaOui[0] ||
			   prWfaIE->aucOui[1] != aucWfaOui[1] || prWfaIE->aucOui[2] != aucWfaOui[2]) {
			break;
		}

		*pucOuiType = prWfaIE->ucOuiType;
		WLAN_GET_FIELD_16(&prWfaIE->aucOuiSubTypeVersion[0], pu2SubTypeVersion);

		return TRUE;
	} while (FALSE);

	return FALSE;

}				

#if CFG_SUPPORT_AAA
void rsnParserCheckForRSNCCMPPSK(P_ADAPTER_T prAdapter, P_RSN_INFO_ELEM_T prIe, PUINT_16 pu2StatusCode)
{

	RSN_INFO_T rRsnIe;

	ASSERT(prAdapter);
	ASSERT(prIe);
	ASSERT(pu2StatusCode);

	*pu2StatusCode = STATUS_CODE_INVALID_INFO_ELEMENT;

	if (rsnParseRsnIE(prAdapter, prIe, &rRsnIe)) {
		if ((rRsnIe.u4PairwiseKeyCipherSuiteCount != 1)
		    || (rRsnIe.au4PairwiseKeyCipherSuite[0] != RSN_CIPHER_SUITE_CCMP)) {
			*pu2StatusCode = STATUS_CODE_INVALID_PAIRWISE_CIPHER;
			return;
		}
		if (rRsnIe.u4GroupKeyCipherSuite != RSN_CIPHER_SUITE_CCMP) {
			*pu2StatusCode = STATUS_CODE_INVALID_GROUP_CIPHER;
			return;
		}
		if ((rRsnIe.u4AuthKeyMgtSuiteCount != 1) || (rRsnIe.au4AuthKeyMgtSuite[0] != RSN_AKM_SUITE_PSK)) {
			*pu2StatusCode = STATUS_CODE_INVALID_AKMP;
			return;
		}

		DBGLOG(RSN, TRACE, "RSN with CCMP-PSK\n");
		*pu2StatusCode = WLAN_STATUS_SUCCESS;
	}

}
#endif

VOID rsnGenMicErrorEvent(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgFlags)
{
	P_PARAM_AUTH_EVENT_T prAuthEvent;

	DEBUGFUNC("rsnGenMicErrorEvent");

	prAuthEvent = (P_PARAM_AUTH_EVENT_T) prAdapter->aucIndicationEventBuffer;

	
	prAuthEvent->rStatus.eStatusType = ENUM_STATUS_TYPE_AUTHENTICATION;

	
	prAuthEvent->arRequest[0].u4Length = sizeof(PARAM_AUTH_REQUEST_T);
	kalMemCopy((PVOID) prAuthEvent->arRequest[0].arBssid,
		   (PVOID) prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX].aucBSSID, MAC_ADDR_LEN);

	if (fgFlags == TRUE)
		prAuthEvent->arRequest[0].u4Flags = PARAM_AUTH_REQUEST_GROUP_ERROR;
	else
		prAuthEvent->arRequest[0].u4Flags = PARAM_AUTH_REQUEST_PAIRWISE_ERROR;

	kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
				     WLAN_STATUS_MEDIA_SPECIFIC_INDICATION,
				     (PVOID) prAuthEvent,
				     sizeof(PARAM_STATUS_INDICATION_T) + sizeof(PARAM_AUTH_REQUEST_T));

}				

VOID rsnTkipHandleMICFailure(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta, IN BOOLEAN fgErrorKeyType)
{
	
	

	DEBUGFUNC("rsnTkipHandleMICFailure");

	ASSERT(prAdapter);
#if 1
	rsnGenMicErrorEvent(prAdapter,  fgErrorKeyType);

	nicConfigPowerSaveProfile(prAdapter, NETWORK_TYPE_AIS_INDEX, Param_PowerModeCAM, FALSE);

	
	DBGLOG(RSN, INFO, "Generate TKIP MIC error event (type: 0%d)\n", fgErrorKeyType);
#else
	ASSERT(prSta);

	prAisSpecBssInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;

	
	GET_CURRENT_SYSTIME(&u4RsnaCurrentMICFailTime);

	
	DBGLOG(RSN, INFO, "Generate TKIP MIC error event (type: 0%d)\n", fgErrorKeyType);

	if (prAisSpecBssInfo->u4RsnaLastMICFailTime != 0 &&
	    !CHECK_FOR_TIMEOUT(u4RsnaCurrentMICFailTime,
			       prAisSpecBssInfo->u4RsnaLastMICFailTime, SEC_TO_SYSTIME(TKIP_COUNTERMEASURE_SEC))) {

		DBGLOG(RSN, INFO, "Start blocking traffic!\n");
		rsnGenMicErrorEvent(prAdapter,  fgErrorKeyType);

		secFsmEventStartCounterMeasure(prAdapter, prSta);
	} else {
		rsnGenMicErrorEvent(prAdapter,  fgErrorKeyType);
		DBGLOG(RSN, INFO, "First TKIP MIC error!\n");
	}

	COPY_SYSTIME(prAisSpecBssInfo->u4RsnaLastMICFailTime, u4RsnaCurrentMICFailTime);
#endif
}				

VOID rsnSelectPmkidCandidateList(IN P_ADAPTER_T prAdapter, IN P_BSS_DESC_T prBssDesc)
{
	P_CONNECTION_SETTINGS_T prConnSettings;
	P_AIS_BSS_INFO_T prAisBssInfo;

	DEBUGFUNC("rsnSelectPmkidCandidateList");

	ASSERT(prBssDesc);

	prConnSettings = &prAdapter->rWifiVar.rConnSettings;
	prAisBssInfo = &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX];

	
	
	

	if (UNEQUAL_SSID(prBssDesc->aucSSID, prBssDesc->ucSSIDLen,
			prConnSettings->aucSSID, prConnSettings->ucSSIDLen)) {
		DBGLOG(RSN, TRACE, "-- SSID not matched\n");
		return;
	}
#if 0
	if ((prBssDesc->u2BSSBasicRateSet &
	     ~(rPhyAttributes[prAisBssInfo->ePhyType].u2SupportedRateSet)) || prBssDesc->fgIsUnknownBssBasicRate) {
		DBGLOG(RSN, TRACE, "-- Rate set not matched\n");
		return;
	}

	if (
		   prBssDesc->u4RsnSelectedGroupCipher != prAisBssInfo->u4RsnSelectedGroupCipher	
) {
		DBGLOG(RSN, TRACE, "-- Encrypt status not matched for PMKID\n");
		return;
	}
#endif

	rsnUpdatePmkidCandidateList(prAdapter, prBssDesc);

}				

VOID rsnUpdatePmkidCandidateList(IN P_ADAPTER_T prAdapter, IN P_BSS_DESC_T prBssDesc)
{
	UINT_32 i;
	P_CONNECTION_SETTINGS_T prConnSettings;
	P_AIS_SPECIFIC_BSS_INFO_T prAisSpecBssInfo;

	DEBUGFUNC("rsnUpdatePmkidCandidateList");

	ASSERT(prBssDesc);

	prConnSettings = &prAdapter->rWifiVar.rConnSettings;
	prAisSpecBssInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;

	if (UNEQUAL_SSID(prBssDesc->aucSSID, prBssDesc->ucSSIDLen,
		prConnSettings->aucSSID, prConnSettings->ucSSIDLen)) {
		DBGLOG(RSN, TRACE, "-- SSID not matched\n");
		return;
	}

	for (i = 0; i < CFG_MAX_PMKID_CACHE; i++) {
		if (EQUAL_MAC_ADDR(prBssDesc->aucBSSID, prAisSpecBssInfo->arPmkidCandicate[i].aucBssid))
			return;
	}

	if ((prAisSpecBssInfo->u4PmkidCandicateCount + 1) > CFG_MAX_PMKID_CACHE)
		prAisSpecBssInfo->u4PmkidCandicateCount--;

	i = prAisSpecBssInfo->u4PmkidCandicateCount;

	COPY_MAC_ADDR((PVOID) prAisSpecBssInfo->arPmkidCandicate[i].aucBssid, (PVOID) prBssDesc->aucBSSID);

	if (prBssDesc->u2RsnCap & MASK_RSNIE_CAP_PREAUTH) {
		prAisSpecBssInfo->arPmkidCandicate[i].u4PreAuthFlags = 1;
		DBGLOG(RSN, TRACE, "Add %pM with pre-auth to candidate list\n",
				    (prAisSpecBssInfo->arPmkidCandicate[i].aucBssid));
	} else {
		prAisSpecBssInfo->arPmkidCandicate[i].u4PreAuthFlags = 0;
		DBGLOG(RSN, TRACE, "Add %pM without pre-auth to candidate list\n",
				    (prAisSpecBssInfo->arPmkidCandicate[i].aucBssid));
	}

	prAisSpecBssInfo->u4PmkidCandicateCount++;

}				

BOOLEAN rsnSearchPmkidEntry(IN P_ADAPTER_T prAdapter, IN PUINT_8 pucBssid, OUT PUINT_32 pu4EntryIndex)
{
	UINT_32 i;
	P_AIS_SPECIFIC_BSS_INFO_T prAisSpecBssInfo;

	DEBUGFUNC("rsnSearchPmkidEntry");

	ASSERT(pucBssid);
	ASSERT(pu4EntryIndex);

	prAisSpecBssInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;

	if (prAisSpecBssInfo->u4PmkidCacheCount > CFG_MAX_PMKID_CACHE)
		return FALSE;

	ASSERT(prAisSpecBssInfo->u4PmkidCacheCount <= CFG_MAX_PMKID_CACHE);

	
	for (i = 0; i < prAisSpecBssInfo->u4PmkidCacheCount; i++) {
		if (!kalMemCmp(prAisSpecBssInfo->arPmkidCache[i].rBssidInfo.arBSSID, pucBssid, MAC_ADDR_LEN))
			break;
	}

	
	if (i < prAisSpecBssInfo->u4PmkidCacheCount) {
		*pu4EntryIndex = i;

		return TRUE;
	}

	return FALSE;
}				

BOOLEAN rsnCheckPmkidCandicate(IN P_ADAPTER_T prAdapter)
{
	P_AIS_SPECIFIC_BSS_INFO_T prAisSpecBssInfo;
	UINT_32 i;		
	UINT_32 j;		
	BOOLEAN status = FALSE;

	DEBUGFUNC("rsnCheckPmkidCandicate");

	prAisSpecBssInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;

	
	for (i = 0; i < prAisSpecBssInfo->u4PmkidCandicateCount; i++) {
		for (j = 0; j < prAisSpecBssInfo->u4PmkidCacheCount; j++) {
			if (!kalMemCmp(prAisSpecBssInfo->arPmkidCache[j].rBssidInfo.arBSSID,
				       prAisSpecBssInfo->arPmkidCandicate[i].aucBssid, MAC_ADDR_LEN)) {
				break;
			}
		}

		
		if (j == prAisSpecBssInfo->u4PmkidCacheCount
		    && prAisSpecBssInfo->u4PmkidCacheCount < CFG_MAX_PMKID_CACHE) {
			DBGLOG(RSN, TRACE,
			       "Add %pM to PMKID cache!!\n",
				(prAisSpecBssInfo->arPmkidCandicate[i].aucBssid));
			kalMemCopy((PVOID) prAisSpecBssInfo->
				   arPmkidCache[prAisSpecBssInfo->u4PmkidCacheCount].rBssidInfo.arBSSID,
				   (PVOID) prAisSpecBssInfo->arPmkidCandicate[i].aucBssid, MAC_ADDR_LEN);
			prAisSpecBssInfo->arPmkidCache[prAisSpecBssInfo->u4PmkidCacheCount].fgPmkidExist = FALSE;
			prAisSpecBssInfo->u4PmkidCacheCount++;

			status = TRUE;
		}
	}

	return status;
}				

VOID rsnIndicatePmkidCand(IN P_ADAPTER_T prAdapter, IN ULONG ulParm)
{
	DBGLOG(RSN, EVENT, "Security - Time to indicate the PMKID cand.\n");


	if (prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX].eConnectionState == PARAM_MEDIA_STATE_CONNECTED &&
	    prAdapter->rWifiVar.rConnSettings.eAuthMode == AUTH_MODE_WPA2) {
		rsnGeneratePmkidIndication(prAdapter);
	}

}				

VOID rsnCheckPmkidCache(IN P_ADAPTER_T prAdapter, IN P_BSS_DESC_T prBss)
{
	P_AIS_BSS_INFO_T prAisBssInfo;
	P_AIS_SPECIFIC_BSS_INFO_T prAisSpecBssInfo;
	P_CONNECTION_SETTINGS_T prConnSettings;

	DEBUGFUNC("rsnCheckPmkidCandicate");

	ASSERT(prBss);

	prConnSettings = &prAdapter->rWifiVar.rConnSettings;
	prAisBssInfo = &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX];
	prAisSpecBssInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;

	if ((prAisBssInfo->eConnectionState == PARAM_MEDIA_STATE_CONNECTED) &&
	    (prConnSettings->eAuthMode == AUTH_MODE_WPA2)) {
		rsnSelectPmkidCandidateList(prAdapter, prBss);

		if (rsnCheckPmkidCandicate(prAdapter)) {
			DBGLOG(RSN, TRACE, "Prepare a timer to indicate candidate PMKID Candidate\n");
			cnmTimerStopTimer(prAdapter, &prAisSpecBssInfo->rPreauthenticationTimer);
			cnmTimerStartTimer(prAdapter, &prAisSpecBssInfo->rPreauthenticationTimer,
					   SEC_TO_MSEC(WAIT_TIME_IND_PMKID_CANDICATE_SEC));
		}
	}
}

VOID rsnGeneratePmkidIndication(IN P_ADAPTER_T prAdapter)
{
	P_PARAM_STATUS_INDICATION_T prStatusEvent;
	P_PARAM_PMKID_CANDIDATE_LIST_T prPmkidEvent;
	P_AIS_SPECIFIC_BSS_INFO_T prAisSpecificBssInfo;
	UINT_8 i, j = 0, count = 0;
	UINT_32 u4LenOfUsedBuffer;

	DEBUGFUNC("rsnGeneratePmkidIndication");

	ASSERT(prAdapter);

	prStatusEvent = (P_PARAM_STATUS_INDICATION_T) prAdapter->aucIndicationEventBuffer;

	
	prStatusEvent->eStatusType = ENUM_STATUS_TYPE_CANDIDATE_LIST;
	ASSERT(prStatusEvent);

	prPmkidEvent = (P_PARAM_PMKID_CANDIDATE_LIST_T) (&prStatusEvent->eStatusType + 1);
	ASSERT(prPmkidEvent);

	prAisSpecificBssInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;
	ASSERT(prAisSpecificBssInfo);

	for (i = 0; i < prAisSpecificBssInfo->u4PmkidCandicateCount; i++) {
		for (j = 0; j < prAisSpecificBssInfo->u4PmkidCacheCount; j++) {
			if (EQUAL_MAC_ADDR(prAisSpecificBssInfo->arPmkidCache[j].rBssidInfo.arBSSID,
					   prAisSpecificBssInfo->arPmkidCandicate[i].aucBssid) &&
			    (prAisSpecificBssInfo->arPmkidCache[j].fgPmkidExist == TRUE)) {
				break;
			}
		}
		if (count >= CFG_MAX_PMKID_CACHE)
			break;

		if (j == prAisSpecificBssInfo->u4PmkidCacheCount) {
			kalMemCopy((PVOID) prPmkidEvent->arCandidateList[count].arBSSID,
				   (PVOID) prAisSpecificBssInfo->arPmkidCandicate[i].aucBssid, PARAM_MAC_ADDR_LEN);
			prPmkidEvent->arCandidateList[count].u4Flags =
			    prAisSpecificBssInfo->arPmkidCandicate[i].u4PreAuthFlags;
			DBGLOG(RSN, TRACE, "%pM %d\n", (prPmkidEvent->arCandidateList[count].arBSSID),
					    (UINT_32) prPmkidEvent->arCandidateList[count].u4Flags);
			count++;
		}
	}

	
	prPmkidEvent->u4Version = 1;
	prPmkidEvent->u4NumCandidates = count;
	DBGLOG(RSN, TRACE, "rsnGeneratePmkidIndication #%d\n", (UINT_32) prPmkidEvent->u4NumCandidates);
	u4LenOfUsedBuffer = sizeof(ENUM_STATUS_TYPE_T) + (2 * sizeof(UINT_32)) +
	    (count * sizeof(PARAM_PMKID_CANDIDATE_T));
	

	kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
				     WLAN_STATUS_MEDIA_SPECIFIC_INDICATION,
				     (PVOID) prAdapter->aucIndicationEventBuffer, u4LenOfUsedBuffer);

}				
#endif

#if CFG_SUPPORT_WPS2
VOID rsnGenerateWSCIE(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo)
{
	PUINT_8 pucBuffer;

	ASSERT(prAdapter);
	ASSERT(prMsduInfo);

	if (prMsduInfo->ucNetworkType != NETWORK_TYPE_AIS_INDEX)
		return;

	pucBuffer = (PUINT_8) ((ULONG) prMsduInfo->prPacket + (UINT_32) prMsduInfo->u2FrameLength);

	
	if (prAdapter->prGlueInfo->u2WSCAssocInfoIELen) {
		kalMemCopy(pucBuffer, &prAdapter->prGlueInfo->aucWSCAssocInfoIE,
			   prAdapter->prGlueInfo->u2WSCAssocInfoIELen);
		prMsduInfo->u2FrameLength += prAdapter->prGlueInfo->u2WSCAssocInfoIELen;
	}

}
#endif

#if CFG_SUPPORT_802_11W

UINT_32 rsnCheckBipKeyInstalled(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prStaRec)
{
	if (prStaRec && prStaRec->ucNetTypeIndex == (UINT_8) NETWORK_TYPE_AIS_INDEX)
		return prAdapter->rWifiVar.rAisSpecificBssInfo.fgBipKeyInstalled;
	else
		return FALSE;
}

UINT_8 rsnCheckSaQueryTimeout(IN P_ADAPTER_T prAdapter)
{
	P_AIS_SPECIFIC_BSS_INFO_T prBssSpecInfo;
	UINT_32 now;

	prBssSpecInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;
	ASSERT(prBssSpecInfo);

	GET_CURRENT_SYSTIME(&now);

	if (CHECK_FOR_TIMEOUT(now, prBssSpecInfo->u4SaQueryStart, TU_TO_MSEC(1000))) {
		LOG_FUNC("association SA Query timed out\n");

		prBssSpecInfo->ucSaQueryTimedOut = 1;
		kalMemFree(prBssSpecInfo->pucSaQueryTransId, VIR_MEM_TYPE,
			   prBssSpecInfo->u4SaQueryCount * ACTION_SA_QUERY_TR_ID_LEN);
		prBssSpecInfo->pucSaQueryTransId = NULL;
		prBssSpecInfo->u4SaQueryCount = 0;
		cnmTimerStopTimer(prAdapter, &prBssSpecInfo->rSaQueryTimer);
		
		DBGLOG(RSN, TRACE, "DisBy11w\n");
		kalIndicateStatusAndComplete(prAdapter->prGlueInfo, WLAN_STATUS_MEDIA_DISCONNECT, NULL, 0);

		return 1;
	}

	return 0;
}

void rsnStartSaQueryTimer(IN P_ADAPTER_T prAdapter)
{
	P_BSS_INFO_T prBssInfo;
	P_AIS_SPECIFIC_BSS_INFO_T prBssSpecInfo;
	P_MSDU_INFO_T prMsduInfo;
	P_ACTION_SA_QUERY_FRAME prTxFrame;
	UINT_16 u2PayloadLen;
	PUINT_8 pucTmp = NULL;
	UINT_8 ucTransId[ACTION_SA_QUERY_TR_ID_LEN];

	prBssInfo = &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX];
	ASSERT(prBssInfo);

	prBssSpecInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;
	ASSERT(prBssSpecInfo);

	LOG_FUNC("MFP: Start Sa Query\n");

	if (prBssSpecInfo->u4SaQueryCount > 0 && rsnCheckSaQueryTimeout(prAdapter)) {
		LOG_FUNC("MFP: u4SaQueryCount count =%d\n", prBssSpecInfo->u4SaQueryCount);
		return;
	}

	prMsduInfo = (P_MSDU_INFO_T) cnmMgtPktAlloc(prAdapter, MAC_TX_RESERVED_FIELD + PUBLIC_ACTION_MAX_LEN);

	if (!prMsduInfo)
		return;

	prTxFrame = (P_ACTION_SA_QUERY_FRAME)
	    ((ULONG) (prMsduInfo->prPacket) + MAC_TX_RESERVED_FIELD);

	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;
	prTxFrame->u2FrameCtrl |= MASK_FC_PROTECTED_FRAME;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prBssInfo->aucBSSID);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	prTxFrame->ucCategory = CATEGORY_SA_QUERT_ACTION;
	prTxFrame->ucAction = ACTION_SA_QUERY_REQUEST;

	if (prBssSpecInfo->u4SaQueryCount == 0)
		GET_CURRENT_SYSTIME(&prBssSpecInfo->u4SaQueryStart);

	if (prBssSpecInfo->u4SaQueryCount) {
		pucTmp = kalMemAlloc(prBssSpecInfo->u4SaQueryCount * ACTION_SA_QUERY_TR_ID_LEN, VIR_MEM_TYPE);
		if (!pucTmp) {
			DBGLOG(RSN, ERROR, "MFP: Fail to alloc tmp buffer for backup sa query id\n");
			return;
		}
		kalMemCopy(pucTmp, prBssSpecInfo->pucSaQueryTransId,
			   prBssSpecInfo->u4SaQueryCount * ACTION_SA_QUERY_TR_ID_LEN);
	}

	kalMemFree(prBssSpecInfo->pucSaQueryTransId, VIR_MEM_TYPE,
		   prBssSpecInfo->u4SaQueryCount * ACTION_SA_QUERY_TR_ID_LEN);

	ucTransId[0] = (UINT_8) (kalRandomNumber() & 0xFF);
	ucTransId[1] = (UINT_8) (kalRandomNumber() & 0xFF);

	kalMemCopy(prTxFrame->ucTransId, ucTransId, ACTION_SA_QUERY_TR_ID_LEN);

	prBssSpecInfo->u4SaQueryCount++;

	prBssSpecInfo->pucSaQueryTransId =
	    kalMemAlloc(prBssSpecInfo->u4SaQueryCount * ACTION_SA_QUERY_TR_ID_LEN, VIR_MEM_TYPE);
	if (!prBssSpecInfo->pucSaQueryTransId) {
		DBGLOG(RSN, ERROR, "MFP: Fail to alloc buffer for sa query id list\n");
		return;
	}

	if (pucTmp) {
		kalMemCopy(prBssSpecInfo->pucSaQueryTransId, pucTmp,
			   (prBssSpecInfo->u4SaQueryCount - 1) * ACTION_SA_QUERY_TR_ID_LEN);
		kalMemCopy(&prBssSpecInfo->pucSaQueryTransId
			   [(prBssSpecInfo->u4SaQueryCount - 1) * ACTION_SA_QUERY_TR_ID_LEN], ucTransId,
			   ACTION_SA_QUERY_TR_ID_LEN);
		kalMemFree(pucTmp, VIR_MEM_TYPE, (prBssSpecInfo->u4SaQueryCount - 1) * ACTION_SA_QUERY_TR_ID_LEN);
	} else {
		kalMemCopy(prBssSpecInfo->pucSaQueryTransId, ucTransId, ACTION_SA_QUERY_TR_ID_LEN);
	}

	u2PayloadLen = 2 + ACTION_SA_QUERY_TR_ID_LEN;

	
	prMsduInfo->ucPacketType = HIF_TX_PACKET_TYPE_MGMT;	
	prMsduInfo->ucStaRecIndex = prBssInfo->prStaRecOfAP->ucIndex;
	prMsduInfo->ucNetworkType = prBssInfo->ucNetTypeIndex;
	prMsduInfo->ucMacHeaderLength = WLAN_MAC_MGMT_HEADER_LEN;
	prMsduInfo->fgIs802_1x = FALSE;
	prMsduInfo->fgIs802_11 = TRUE;
	prMsduInfo->u2FrameLength = WLAN_MAC_MGMT_HEADER_LEN + u2PayloadLen;
	prMsduInfo->ucTxSeqNum = nicIncreaseTxSeqNum(prAdapter);
	prMsduInfo->pfTxDoneHandler = NULL;
	prMsduInfo->fgIsBasicRate = FALSE;

	
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

	DBGLOG(RSN, TRACE,
	       "Set SA Query timer %d (%d sec)\n", prBssSpecInfo->u4SaQueryCount, prBssInfo->u2ObssScanInterval);

	cnmTimerStartTimer(prAdapter, &prBssSpecInfo->rSaQueryTimer, TU_TO_MSEC(201));

}

void rsnStartSaQuery(IN P_ADAPTER_T prAdapter)
{
	rsnStartSaQueryTimer(prAdapter);
}

void rsnStopSaQuery(IN P_ADAPTER_T prAdapter)
{
	P_AIS_SPECIFIC_BSS_INFO_T prBssSpecInfo;

	prBssSpecInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;
	ASSERT(prBssSpecInfo);

	cnmTimerStopTimer(prAdapter, &prBssSpecInfo->rSaQueryTimer);
	kalMemFree(prBssSpecInfo->pucSaQueryTransId, VIR_MEM_TYPE,
		   prBssSpecInfo->u4SaQueryCount * ACTION_SA_QUERY_TR_ID_LEN);
	prBssSpecInfo->pucSaQueryTransId = NULL;
	prBssSpecInfo->u4SaQueryCount = 0;
}

void rsnSaQueryRequest(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb)
{
	P_BSS_INFO_T prBssInfo;
	P_MSDU_INFO_T prMsduInfo;
	P_ACTION_SA_QUERY_FRAME prRxFrame = NULL;
	UINT_16 u2PayloadLen;
	P_STA_RECORD_T prStaRec;
	P_ACTION_SA_QUERY_FRAME prTxFrame;

	prBssInfo = &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX];
	ASSERT(prBssInfo);

	prRxFrame = (P_ACTION_SA_QUERY_FRAME) prSwRfb->pvHeader;
	if (!prRxFrame)
		return;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);

	DBGLOG(RSN, TRACE, "IEEE 802.11: Received SA Query Request from %pM\n", prStaRec->aucMacAddr);

	DBGLOG_MEM8(RSN, TRACE, prRxFrame->ucTransId, ACTION_SA_QUERY_TR_ID_LEN);

	if (kalGetMediaStateIndicated(prAdapter->prGlueInfo) == PARAM_MEDIA_STATE_DISCONNECTED) {
		DBGLOG(RSN, TRACE, "IEEE 802.11: Ignore SA Query Request from unassociated STA %pM\n",
				    prStaRec->aucMacAddr);
		return;
	}
	DBGLOG(RSN, TRACE, "IEEE 802.11: Sending SA Query Response to %pM\n", prStaRec->aucMacAddr);

	prMsduInfo = (P_MSDU_INFO_T) cnmMgtPktAlloc(prAdapter, MAC_TX_RESERVED_FIELD + PUBLIC_ACTION_MAX_LEN);

	if (!prMsduInfo)
		return;

	prTxFrame = (P_ACTION_SA_QUERY_FRAME)
	    ((ULONG) (prMsduInfo->prPacket) + MAC_TX_RESERVED_FIELD);

	prTxFrame->u2FrameCtrl = MAC_FRAME_ACTION;
	
	prTxFrame->u2FrameCtrl |= MASK_FC_PROTECTED_FRAME;

	COPY_MAC_ADDR(prTxFrame->aucDestAddr, prBssInfo->aucBSSID);
	COPY_MAC_ADDR(prTxFrame->aucSrcAddr, prBssInfo->aucOwnMacAddr);
	COPY_MAC_ADDR(prTxFrame->aucBSSID, prBssInfo->aucBSSID);

	prTxFrame->ucCategory = CATEGORY_SA_QUERT_ACTION;
	prTxFrame->ucAction = ACTION_SA_QUERY_RESPONSE;

	kalMemCopy(prTxFrame->ucTransId, prRxFrame->ucTransId, ACTION_SA_QUERY_TR_ID_LEN);

	u2PayloadLen = 2 + ACTION_SA_QUERY_TR_ID_LEN;

	
	prMsduInfo->ucPacketType = HIF_TX_PACKET_TYPE_MGMT;	
	prMsduInfo->ucStaRecIndex = prBssInfo->prStaRecOfAP->ucIndex;
	prMsduInfo->ucNetworkType = prBssInfo->ucNetTypeIndex;
	prMsduInfo->ucMacHeaderLength = WLAN_MAC_MGMT_HEADER_LEN;
	prMsduInfo->fgIs802_1x = FALSE;
	prMsduInfo->fgIs802_11 = TRUE;
	prMsduInfo->u2FrameLength = WLAN_MAC_MGMT_HEADER_LEN + u2PayloadLen;
	prMsduInfo->ucTxSeqNum = nicIncreaseTxSeqNum(prAdapter);
	prMsduInfo->pfTxDoneHandler = NULL;
	prMsduInfo->fgIsBasicRate = FALSE;

	
	nicTxEnqueueMsdu(prAdapter, prMsduInfo);

}

void rsnSaQueryAction(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb)
{
	P_AIS_SPECIFIC_BSS_INFO_T prBssSpecInfo;
	P_ACTION_SA_QUERY_FRAME prRxFrame;
	P_STA_RECORD_T prStaRec;
	UINT_32 i;

	prBssSpecInfo = &prAdapter->rWifiVar.rAisSpecificBssInfo;
	ASSERT(prBssSpecInfo);

	prRxFrame = (P_ACTION_SA_QUERY_FRAME) prSwRfb->pvHeader;
	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);

	if (prSwRfb->u2PacketLen < ACTION_SA_QUERY_TR_ID_LEN) {
		DBGLOG(RSN, TRACE, "IEEE 802.11: Too short SA Query Action frame (len=%u)\n",
				    prSwRfb->u2PacketLen);
		return;
	}

	if (prRxFrame->ucAction == ACTION_SA_QUERY_REQUEST) {
		rsnSaQueryRequest(prAdapter, prSwRfb);
		return;
	}

	if (prRxFrame->ucAction != ACTION_SA_QUERY_RESPONSE) {
		DBGLOG(RSN, TRACE, "IEEE 802.11: Unexpected SA Query " "Action %d\n", prRxFrame->ucAction);
		return;
	}

	DBGLOG(RSN, TRACE, "IEEE 802.11: Received SA Query Response from %pM\n", prStaRec->aucMacAddr);

	DBGLOG_MEM8(RSN, TRACE, prRxFrame->ucTransId, ACTION_SA_QUERY_TR_ID_LEN);

	

	for (i = 0; i < prBssSpecInfo->u4SaQueryCount; i++) {
		if (kalMemCmp(prBssSpecInfo->pucSaQueryTransId +
			      i * ACTION_SA_QUERY_TR_ID_LEN, prRxFrame->ucTransId, ACTION_SA_QUERY_TR_ID_LEN) == 0)
			break;
	}

	if (i >= prBssSpecInfo->u4SaQueryCount) {
		DBGLOG(RSN, TRACE, "IEEE 802.11: No matching SA Query " "transaction identifier found\n");
		return;
	}

	DBGLOG(RSN, TRACE, "Reply to pending SA Query received\n");

	rsnStopSaQuery(prAdapter);
}

BOOLEAN rsnCheckRxMgmt(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfb, IN UINT_8 ucSubtype)
{
	P_HIF_RX_HEADER_T prHifRxHdr;
	BOOLEAN fgUnicast = TRUE;
	BOOLEAN fgRobustAction = FALSE;

	prHifRxHdr = prSwRfb->prHifRxHdr;

	if ((HIF_RX_HDR_GET_NETWORK_IDX(prHifRxHdr) == NETWORK_TYPE_AIS_INDEX) &&
	    prAdapter->rWifiVar.rAisSpecificBssInfo.fgMgmtProtection ) {

		P_WLAN_ASSOC_REQ_FRAME_T prAssocReqFrame;

		prAssocReqFrame = (P_WLAN_ASSOC_REQ_FRAME_T) prSwRfb->pvHeader;

		if (prAssocReqFrame->aucDestAddr[0] & BIT(0))
			fgUnicast = FALSE;

		LOG_FUNC("QM RX MGT: rsnCheckRxMgmt = %d 0x%x %d ucSubtype=%x\n", fgUnicast, prHifRxHdr->ucReserved,
			 (prHifRxHdr->ucReserved & CONTROL_FLAG_UC_MGMT_NO_ENC), ucSubtype);

		if (prHifRxHdr->ucReserved & CONTROL_FLAG_UC_MGMT_NO_ENC) {
			
			
			if (ucSubtype == 13 ) {
				UINT_8 ucAction = *prSwRfb->pucRecvBuff;

				if (ucAction != CATEGORY_PUBLIC_ACTION && ucAction != CATEGORY_HT_ACTION) {
#if DBG && CFG_RX_PKTS_DUMP
					LOG_FUNC("QM RX MGT: UnProtected Robust Action frame = %d\n", ucAction);
#endif
					fgRobustAction = TRUE;
					return TRUE;
				}
			}
			if (fgUnicast && ((ucSubtype == 10 )
					  || (ucSubtype == 12 ))) {
				LOG_FUNC("QM RX MGT: rsnStartSaQuery\n");
				
				rsnStartSaQuery(prAdapter);
				return TRUE;
			}
		}
#if 0
		else {
			if (fgUnicast && ((ucSubtype == MAC_FRAME_DISASSOC) || (ucSubtype == MAC_FRAME_DEAUTH))) {
				
				
				
				
				
			}
		}
#endif
	}
	return FALSE;
}
#endif

#if CFG_SUPPORT_DETECT_SECURITY_MODE_CHANGE
static BOOLEAN rsnCheckWpaRsnInfo(P_BSS_INFO_T prBss, P_RSN_INFO_T prWpaRsnInfo)
{
	UINT_32 i = 0;

	if (prWpaRsnInfo->u4GroupKeyCipherSuite != prBss->u4RsnSelectedGroupCipher) {
		DBGLOG(RSN, INFO, "GroupCipherSuite change, old=0x%04x, new=0x%04x\n",
				   prBss->u4RsnSelectedGroupCipher, prWpaRsnInfo->u4GroupKeyCipherSuite);
		return TRUE;
	}
	for (; i < prWpaRsnInfo->u4AuthKeyMgtSuiteCount; i++)
		if (prBss->u4RsnSelectedAKMSuite == prWpaRsnInfo->au4AuthKeyMgtSuite[i])
			break;
	if (i == prWpaRsnInfo->u4AuthKeyMgtSuiteCount) {
		DBGLOG(RSN, INFO, "KeyMgmt change, not find 0x%04x in new beacon\n", prBss->u4RsnSelectedAKMSuite);
		return TRUE;
	}

	for (i = 0; i < prWpaRsnInfo->u4PairwiseKeyCipherSuiteCount; i++)
		if (prBss->u4RsnSelectedPairwiseCipher == prWpaRsnInfo->au4PairwiseKeyCipherSuite[i])
			break;
	if (i == prWpaRsnInfo->u4PairwiseKeyCipherSuiteCount) {
		DBGLOG(RSN, INFO, "Pairwise Cipher change, not find 0x%04x in new beacon\n",
				   prBss->u4RsnSelectedPairwiseCipher);
		return TRUE;
	}

	return FALSE;
}

BOOLEAN rsnCheckSecurityModeChanged(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo, P_BSS_DESC_T prBssDesc)
{
	ENUM_PARAM_AUTH_MODE_T eAuthMode = prAdapter->rWifiVar.rConnSettings.eAuthMode;

	switch (eAuthMode) {
	case AUTH_MODE_OPEN:	
		if ((prBssDesc->u2CapInfo & CAP_INFO_PRIVACY) && !prAdapter->prGlueInfo->rWpaInfo.fgPrivacyInvoke) {
			DBGLOG(RSN, INFO, "security change, open->privacy\n");
			return TRUE;
		}
		break;
	case AUTH_MODE_SHARED:	
	case AUTH_MODE_AUTO_SWITCH:
		if ((prBssDesc->u2CapInfo & CAP_INFO_PRIVACY) == 0) {
			DBGLOG(RSN, INFO, "security change, WEP->open\n");
			return TRUE;
		} else if (prBssDesc->fgIERSN || prBssDesc->fgIEWPA) {
			DBGLOG(RSN, INFO, "security change, WEP->WPA/WPA2\n");
			return TRUE;
		}
		break;
	case AUTH_MODE_WPA:	
	case AUTH_MODE_WPA_PSK:
	case AUTH_MODE_WPA_NONE:
		if (prBssDesc->fgIEWPA)
			return rsnCheckWpaRsnInfo(prBssInfo, &prBssDesc->rWPAInfo);
		DBGLOG(RSN, INFO, "security change, WPA->%s\n",
				   prBssDesc->fgIERSN ? "WPA2" :
				   (prBssDesc->u2CapInfo & CAP_INFO_PRIVACY ? "WEP" : "OPEN"));
		return TRUE;
	case AUTH_MODE_WPA2:	
	case AUTH_MODE_WPA2_PSK:
		if (prBssDesc->fgIERSN)
			return rsnCheckWpaRsnInfo(prBssInfo, &prBssDesc->rRSNInfo);
		DBGLOG(RSN, INFO, "security change, WPA2->%s\n",
				   prBssDesc->fgIEWPA ? "WPA" :
				   (prBssDesc->u2CapInfo & CAP_INFO_PRIVACY ? "WEP" : "OPEN"));
		return TRUE;
	default:
		DBGLOG(RSN, WARN, "unknowned eAuthMode=%d\n", eAuthMode);
		break;
	}
	return FALSE;
}
#endif
