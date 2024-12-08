#pragma once

void initOTA();
void OTATickImplied();
void startHandlingOTA();
bool checkIfTimeForOTAHandle();

void publicOTATickImplied(); //
void onPublicOTATimer(); //
void startHandlingPublicOTA(); //
bool checkIfTimeForPublicOTAHandle(); //

void haltPublicOTA();
void resumePublicOTA();