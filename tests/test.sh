#!/bin/sh

curl -v \
	-H "Content-Type: application/json" \
	--data "channel_id=hawos4dqtby53pd64o4a4cmeoo" \
	--data "channel_name=town-square"
	--data "team_domain=someteam"
	--data "team_id=kwoknj9nwpypzgzy78wkw516qe"
	--data "text=rogy%3A%20some%5C%20t%C3%A9xt%20%22here+for+lyfe"
	--data "timestamp=1445532266"
	--data "token=zmigewsanbbsdf59xnmduzypjc"
	--data "trigger_word=some"
	--data "user_id=rnina9994bde8mua79zqcg5hmo"
	--data "user_name=somename" \
	http://localhost:3333/
