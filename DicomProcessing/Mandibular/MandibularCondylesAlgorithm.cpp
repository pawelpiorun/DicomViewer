#include "MandibularCondylesAlgorithm.h"

void MandibularCondylesAlgorithm::DumpIntermediates(const char* outputFolder)
{
	// ensure output folder
	std::cout << "Dump folder: " + std::string(outputFolder) << std::endl;
	auto patientFolder = std::string(outputFolder) + "\\" + input->GetPatientId() + "_" + input->GetPatientName();
	auto studyFolder = patientFolder + "\\" + input->GetImageId();
	itksys::SystemTools::MakeDirectory(studyFolder);

	auto metadataPath = studyFolder + "\\metadata.txt";
	auto stream = std::ofstream(metadataPath);
	input->Print(stream);
	stream.close();

	auto path = studyFolder + "\\" + rightLinesImage->GetFileName() + ".jpg";
	std::cout << "[Mandibular] Saving right lines image to: " + path << std::endl;
	rightLinesImage->SaveCompressed(path.c_str());
	std::cout << "[Mandibular] Saving done." << std::endl;

	path = studyFolder + "\\" + leftLinesImage->GetFileName() + ".jpg";
	std::cout << "[Mandibular] Saving left lines image to: " << path << std::endl;
	leftLinesImage->SaveCompressed(path.c_str());
	std::cout << "[Mandibular] Saving done." << std::endl;

	path = studyFolder + "\\" + upperSkullProjection->GetFileName() + ".jpg";
	std::cout << "[Mandibular] Saving upper skull side projection image to: " << path << std::endl;
	upperSkullProjection->SaveCompressed(path.c_str());
	std::cout << "[Mandibular] Saving done." << std::endl;

	path = studyFolder + "\\" + topCutProjection->GetFileName() + ".jpg";
	std::cout << "[Mandibular] Saving upper skull top projection image to: " << path << std::endl;
	topCutProjection->SaveCompressed(path.c_str());
	std::cout << "[Mandibular] Saving done." << std::endl;

	path = studyFolder + "\\" + topCutFilteredProjection->GetFileName() + ".jpg";
	std::cout << "[Mandibular] Saving upper skull top filtered projection image to: " << path << std::endl;
	topCutFilteredProjection->SaveCompressed(path.c_str());
	std::cout << "[Mandibular] Saving done." << std::endl;

	path = studyFolder + "\\" + archSpacesProjection->GetFileName() + ".jpg";
	std::cout << "[Mandibular] Saving zygomatic archs spaces projection image to: " << path << std::endl;
	archSpacesProjection->SaveCompressed(path.c_str());
	std::cout << "[Mandibular] Saving done." << std::endl;

	path = studyFolder + "\\" + archsProjection->GetFileName() + ".jpg";
	std::cout << "[Mandibular] Saving zygomatic archs projection image to: " << path << std::endl;
	archsProjection->SaveCompressed(path.c_str());
	std::cout << "[Mandibular] Saving done." << std::endl;
}