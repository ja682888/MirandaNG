case MIRANDA_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMiranda", 1))
		return _T("Client Icons/Miranda clients");
	break;
case MIRANDA_VERSION_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMirandaVersion", 1))
		return _T("Client Icons/Miranda clients");
	break;
case MIRANDA_PACKS_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMirandaPacks", 1))
		return _T("Client Icons/Miranda clients/pack overlays");
	break;


case MULTI_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMulti", 1))
		return _T("Client Icons/multi-proto clients");
	break;


case AIM_CASE:
	if ( db_get_b(NULL, "Finger", "GroupAIM", 1))
		return _T("Client Icons/AIM clients");
	break;

case GG_CASE:
	if ( db_get_b(NULL, "Finger", "GroupGG", 1))
		return _T("Client Icons/Gadu-Gadu clients");
	break;

	break;
case ICQ_CASE:
	if ( db_get_b(NULL, "Finger", "GroupICQ", 1))
		return _T("Client Icons/ICQ clients");
	break;

case IRC_CASE:
	if ( db_get_b(NULL, "Finger", "GroupIRC", 1))
		return _T("Client Icons/IRC clients");
	break;

case JABBER_CASE:
	if ( db_get_b(NULL, "Finger", "GroupJabber", 1))
		return _T("Client Icons/Jabber clients");
	break;

case MRA_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMRA", 1))
		return _T("Client Icons/MRA clients");
	break;

case MSN_CASE:
	if ( db_get_b(NULL, "Finger", "GroupMSN", 1))
		return _T("Client Icons/MSN clients");
	break;

case QQ_CASE:
	if ( db_get_b(NULL, "Finger", "GroupQQ", 1))
		return _T("Client Icons/QQ clients");
	break;

case RSS_CASE:
	if ( db_get_b(NULL, "Finger", "GroupRSS", 1))
		return _T("Client Icons/RSS clients");
	break;

case TLEN_CASE:
	if ( db_get_b(NULL, "Finger", "GroupTlen", 1))
		return _T("Client Icons/Tlen clients");
	break;

case WEATHER_CASE:
	if ( db_get_b(NULL, "Finger", "GroupWeather", 1))
		return _T("Client Icons/Weather clients");
	break;

case YAHOO_CASE:
	if ( db_get_b(NULL, "Finger", "GroupYahoo", 1))
		return _T("Client Icons/Yahoo clients");
	break;


case OTHER_PROTOS_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOtherProtos", 1))
		return _T("Client Icons/other protocols");
	break;

case OTHERS_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOthers", 1))
		return _T("Client Icons/other icons");
	break;


case OVERLAYS_RESOURCE_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOverlaysResource", 1))
		return _T("Client Icons/overlays/resource");
	break;

case OVERLAYS_PLATFORM_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOverlaysPlatform", 1))
		return _T("Client Icons/overlays/platform");
	break;

case OVERLAYS_PROTO_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOverlaysProtos", 1))
		return _T("Client Icons/overlays/protocol");
	break;

case OVERLAYS_SECURITY_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOtherProtos", 1))
		return _T("Client Icons/overlays/security");
	break;

/*
case OVERLAYS_RESOURCE_ALT_CASE:
	if ( db_get_b(NULL, "Finger", "GroupOverlays", 1))
		return _T("Client Icons/overlays");
	break;
*/

default:
	return _T("Client Icons");
