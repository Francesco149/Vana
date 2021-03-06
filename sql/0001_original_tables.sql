CREATE TABLE characters (
  ID int(11) NOT NULL auto_increment,
  `name` varchar(12) collate latin1_general_ci default NULL,
  userid int(11) default NULL,
  `level` tinyint(3) unsigned default '1',
  job smallint(6) default '0',
  str smallint(6) default NULL,
  dex smallint(6) default NULL,
  intt smallint(6) default NULL,
  luk smallint(6) default NULL,
  chp smallint(6) default '50',
  mhp smallint(6) default '50',
  cmp smallint(6) default '50',
  mmp smallint(6) default '50',
  ap smallint(6) default '0',
  sp smallint(6) default '0',
  exp int(11) default '0',
  fame smallint(6) default '0',
  map int(11) default '0',
  pos smallint(6) default '0',
  gender tinyint(4) default NULL,
  skin tinyint(4) default NULL,
  eyes int(11) default NULL,
  hair int(11) default NULL,
  mesos int(11) default '0',
  PRIMARY KEY  (ID),
  KEY userid (userid)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

CREATE TABLE equip (
  equipid int(11) default NULL,
  `type` tinyint(4) default NULL,
  charid int(11) default NULL,
  pos smallint(6) default '0',
  slots tinyint(4) default '7',
  scrolls smallint(6) default '0',
  istr smallint(6) default '0',
  idex smallint(6) default '0',
  iint smallint(6) default '0',
  iluk smallint(6) default '0',
  ihp smallint(6) default '0',
  imp smallint(6) default '0',
  iwatk smallint(6) default '0',
  imatk smallint(6) default '0',
  iwdef smallint(6) default '0',
  imdef smallint(6) default '0',
  iacc smallint(6) default '0',
  iavo smallint(6) default '0',
  ihand smallint(6) default '0',
  ispeed smallint(6) default '0',
  ijump smallint(6) default '0',
  KEY charid (charid)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

CREATE TABLE items (
  itemid int(11) default NULL,
  charid int(11) default NULL,
  inv tinyint(4) default NULL,
  pos smallint(6) default NULL,
  amount smallint(6) default NULL,
  KEY charid (charid)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

CREATE TABLE keymap (
  charid int(11) NOT NULL default '0',
  pos0 int(11) default '0',
  pos1 int(11) default '0',
  pos2 int(11) default '2564',
  pos3 int(11) default '3076',
  pos4 int(11) default '3332',
  pos5 int(11) default '4612',
  pos6 int(11) default '5380',
  pos7 int(11) default '0',
  pos8 int(11) default '0',
  pos9 int(11) default '0',
  pos10 int(11) default '0',
  pos11 int(11) default '0',
  pos12 int(11) default '0',
  pos13 int(11) default '0',
  pos14 int(11) default '0',
  pos15 int(11) default '0',
  pos16 int(11) default '2052',
  pos17 int(11) default '1284',
  pos18 int(11) default '4',
  pos19 int(11) default '1028',
  pos20 int(11) default '0',
  pos21 int(11) default '0',
  pos22 int(11) default '0',
  pos23 int(11) default '260',
  pos24 int(11) default '0',
  pos25 int(11) default '4868',
  pos26 int(11) default '3588',
  pos27 int(11) default '3844',
  pos28 int(11) default '0',
  pos29 int(11) default '13317',
  pos30 int(11) default '0',
  pos31 int(11) default '516',
  pos32 int(11) default '0',
  pos33 int(11) default '0',
  pos34 int(11) default '3456',
  pos35 int(11) default '2820',
  pos36 int(11) default '0',
  pos37 int(11) default '772',
  pos38 int(11) default '0',
  pos39 int(11) default '5124',
  pos40 int(11) default '4100',
  pos41 int(11) default '0',
  pos42 int(11) default '0',
  pos43 int(11) default '2308',
  pos44 int(11) default '12805',
  pos45 int(11) default '13061',
  pos46 int(11) default '1540',
  pos47 int(11) default '0',
  pos48 int(11) default '0',
  pos49 int(11) default '0',
  pos50 int(11) default '1796',
  pos51 int(11) default '0',
  pos52 int(11) default '0',
  pos53 int(11) default '0',
  pos54 int(11) default '0',
  pos55 int(11) default '0',
  pos56 int(11) default '13573',
  pos57 int(11) default '0',
  pos58 int(11) default '0',
  pos59 int(11) default '25606',
  pos60 int(11) default '25862',
  pos61 int(11) default '26118',
  pos62 int(11) default '26374',
  pos63 int(11) default '26630',
  pos64 int(11) default '26886',
  pos65 int(11) default '27142',
  pos66 int(11) default '0',
  pos67 int(11) default '0',
  pos68 int(11) default '0',
  pos69 int(11) default '0',
  pos70 int(11) default '0',
  pos71 int(11) default '0',
  pos72 int(11) default '0',
  pos73 int(11) default '0',
  pos74 int(11) default '0',
  pos75 int(11) default '0',
  pos76 int(11) default '0',
  pos77 int(11) default '0',
  pos78 int(11) default '0',
  pos79 int(11) default '0',
  pos80 int(11) default '0',
  pos81 int(11) default '0',
  pos82 int(11) default '0',
  pos83 int(11) default '0',
  pos84 int(11) default '0',
  pos85 int(11) default '0',
  pos86 int(11) default '0',
  pos87 int(11) default '0',
  pos88 int(11) default '0',
  pos89 int(11) default '0',
  PRIMARY KEY  (charid)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

CREATE TABLE skills (
  charid int(11) default NULL,
  skillid int(11) default NULL,
  points int(11) default NULL,
  KEY charid (charid)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;

CREATE TABLE users (
  ID int(11) NOT NULL auto_increment,
  username varchar(20) collate latin1_general_ci default NULL,
  `password` varchar(20) collate latin1_general_ci default NULL,
  pin int(11) default '0',
  gender int(11) default '0',
  gm int(11) default '0',
  PRIMARY KEY  (ID),
  KEY username_password (username,`password`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COLLATE=latin1_general_ci;